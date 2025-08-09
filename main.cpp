#include "main.hpp"

void save_config(map<string, double> args) {
    string config(BASE_CONFIG);
    config = set_args(config, args);
    ofstream file(CONFIG_PATH);
    if (!file.is_open())
        throw runtime_error("Failed to open file: " + CONFIG_PATH);
    file << config.c_str();
}

map<string, double> get_config() {
    map<string, double> args(DEFAULT_ARGS);
    string line;
    ifstream file(CONFIG_PATH);
    if (!file.is_open())
        throw runtime_error("Failed to open file: " + CONFIG_PATH);
    ostringstream l;
    l << file.rdbuf();
    for (auto line : split_string(l.str(), "\n")) {
        vector<string> kv = split_string(line, "=");
        if (kv.size() != 2)
            continue;
        trim(kv[0]);
        args["--" + kv[0]] = atof(kv[1].c_str());
    }
    return args;
}

map<string, double> get_args() {
    return fileExists(CONFIG_PATH) ? get_config() : DEFAULT_ARGS;
}

void print_args(map<string, double> args) {
    for (auto p : args) {
        string arg = my_replace(p.first, "--", "");
        if (arg == "")
            continue;
        printf("%s=%s\n", arg.c_str(), fmtFract(p.second).c_str());
    }
}

string set_args(string base_shader, map<string, double> args) {
    string new_shader(base_shader);
    new_shader = my_replace(new_shader, "TEMPERATURE", fmtFract(args["--temperature"]));
    new_shader = my_replace(new_shader, "BRIGHTNESS", fmtFract(args["--brightness"]));
    new_shader = my_replace(new_shader, "CONTRAST", fmtFract(args["--contrast"]));
    new_shader = my_replace(new_shader, "HUE", fmtFract(args["--hue"]));
    new_shader = my_replace(new_shader, "SATURATION", fmtFract(args["--saturation"]));
    new_shader = my_replace(new_shader, "GAMMA", fmtFract(args["--gamma"]));
    new_shader = my_replace(new_shader, "RED", fmtFract(args["--red"]));
    new_shader = my_replace(new_shader, "GREEN", fmtFract(args["--green"]));
    new_shader = my_replace(new_shader, "BLUE", fmtFract(args["--blue"]));
    return new_shader;
}

void save_shader(string new_shader) {
    ofstream file(SHADER_PATH);
    if (!file.is_open())
        throw runtime_error("Failed to open file: " + SHADER_PATH);
    file << new_shader.c_str();
}

void reload_hyprland() { system("hyprctl reload"); }

int main(int argc, char **argv) {
    map<string, double> args = get_args();

    if (argc == 1) {
        print_args(args);
        save_shader(set_args(BASE_SHADER, args));
        reload_hyprland();
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "--help") || strcmp(argv[1], "help") ||
                      strcmp(argv[1], "-h")))
        return puts(HELP.c_str());

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == 45 && i + 1 < argc) {
            args[argv[i]] = atof(argv[i + 1]);
            if (args[argv[i]] < 0.1 && (strcmp("--brightness", argv[i]) == 0 || strcmp("--gamma", argv[i]) == 0))
                args[argv[i]] = 0.1;
            i++;
        }
    };

    save_shader(set_args(BASE_SHADER, args));
    save_config(args);
    reload_hyprland();
    return 0;
}
