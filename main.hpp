#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <memory>
#include <ostream>
#include <spawn.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;

inline string HOME(getenv("HOME"));
inline string SHADER_PATH = HOME + "/.config/hypr/screen_controller_shader.frag";
inline string CONFIG_PATH = HOME + "/.config/hypr/screen_controller_shader.conf";

template <typename... Args>
std::string string_format(const std::string &format, Args... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0) {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

inline void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

inline void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !isspace(ch);
            }).base(),
            s.end());
}

inline void trim(string &s) {
    rtrim(s);
    ltrim(s);
}

inline void toLower(string my_string) {
    std::transform(my_string.begin(), my_string.end(), my_string.begin(),
                   [](unsigned char c) { return tolower(c); });
}

inline void toUpper(string my_string) {
    std::transform(my_string.begin(), my_string.end(), my_string.begin(),
                   [](unsigned char c) { return toupper(c); });
}

inline bool fileExists(string path) {
    return filesystem::exists(path) && filesystem::is_regular_file(path);
}

inline string my_replace(string str, string from, string to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return "";
    return str.replace(start_pos, from.length(), to);
}

inline vector<string> split_string(string str,
                                   const string &delimiter) {
    vector<string> strings;

    string::size_type pos = 0;
    string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev)) != string::npos) {
        strings.push_back(str.substr(prev, pos - prev));
        prev = pos + delimiter.size();
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}

void print_args(map<string, double> args);
string set_args_f(string str, map<string, double> args);
string set_args(string base_shader, map<string, double> args);

// 1 - 5 numbers of fact
inline string fmtFract(double value) {
    value = round(value * 10e5) / 10e5f;
    string s = format("{:.5f}", value);
    size_t dot = s.find('.');
    for (size_t i = s.size() - 1; i > s.find_last_not_of('0') && i > (dot + 1); i--)
        s.erase(i);
    return s;
}

const map<string, double> DEFAULT_ARGS{
    {"--temperature", 6500},
    {"--brightness", 1.0},
    {"--contrast", 1.0},
    {"--gamma", 1.0},
    {"--saturation", 1.0},
    {"--hue", 0.0},
    {"--red", 1.0},
    {"--green", 1.0},
    {"--blue", 1.0},
};

const string BASE_CONFIG_F = R"(red = %.5f
green = %.5f
blue = %.5f
temperature = %.5f
brightness = %.5f
contrast = %.5f
hue = %.5f
saturation = %.5f
gamma = %.5f
)";

const string BASE_SHADER_F = R"(#version 330 core

precision mediump float;
in vec2 v_texcoord;
uniform sampler2D tex;

vec3 new_color = vec3(%.5f, %.5f, %.5f);

vec3 temperatureToRGB(float temp) {
    temp = clamp(temp, 1000.0, 40000.0) / 100.0;
    vec3 color;
    
    if (temp <= 66.0) color.r = 1.0;
    else color.r = 1.292936 * pow(temp - 60.0, -0.1332047);
    
    if (temp <= 66.0) color.g = 0.39008157 * log(temp) - 0.63184144;
    else color.g = 1.129890 * pow(temp - 60.0, -0.0755148);
    
    if (temp >= 66.0) color.b = 1.0;
    else if (temp <= 19.0) color.b = 0.0;
    else color.b = 0.543206789 * log(temp - 10.0) - 1.19625408;
    
    return clamp(color, 0.0, 1.0);
}

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec4 pixColor = texture2D(tex, v_texcoord);
    vec3 color = pixColor.rgb;
    color *= temperatureToRGB(%.5f);
    color *= %.5f;
    color = mix(vec3(dot(vec3(1.0/3.0), color)), color, %.5f);
    vec3 hsv = rgb2hsv(color);
    hsv.x = fract(hsv.x + %.5f);
    hsv.y *= %.5f;
    color = hsv2rgb(hsv);
    color = pow(color, vec3(1.0 / %.5f));
    color *= new_color;
    color = clamp(color, 0.0, 1.0);
    gl_FragColor = vec4(color, pixColor.a);
}
)";

const string HELP = R"(
Arguments & range
--temperatue 6700.0   ~ 1000-40000
--brightness 1.0      ~ 0.1-2.0
--contrast 0.9        ~ 0.1-2.0
--saturation 1.15     ~ 0.1-2.0
--gamma 1.15          ~ 0.1-3.0
--hue 0.007           ~ -1.0-+1.0
--red 1.0             ~ 1.0-2.0
--green 1.0           ~ 1.0-2.0
--blue 1.0            ~ 1.0-2.0
)";