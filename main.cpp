#include "main.hpp"

extern char **environ;

void reload_hyprland_fast() {
    pid_t pid;
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_addopen(&file_actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&file_actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);

    // hyprctl keyword decoration:screen_shader ~/.config/hypr/screen_controller_shader.frag
    const char *argv[] = {"hyprctl", "keyword", "decoration:screen_shader", SHADER_PATH.c_str(), nullptr};

    posix_spawn(&pid, "/usr/bin/hyprctl", &file_actions, nullptr,
                (char *const *)argv, environ);

    posix_spawn_file_actions_destroy(&file_actions);
}

void save_config(map<string, double> args) {
    ofstream file(CONFIG_PATH);
    if (!file.is_open())
        throw runtime_error("Failed to open file: " + CONFIG_PATH);
    file << set_args_f(BASE_CONFIG_F, args).c_str();
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

string set_args_f(string str, map<string, double> args) {
    return string_format(str, args["--red"], args["--green"], args["--blue"],
                         args["--temperature"], args["--brightness"], args["--contrast"],
                         args["--hue"], args["--saturation"], args["--gamma"]);
}

void save_shader(map<string, double> args) {
    ofstream file(SHADER_PATH);
    if (!file.is_open())
        throw runtime_error("Failed to open file: " + SHADER_PATH);
    file << set_args_f(BASE_SHADER_F, args).c_str();
}

int main(int argc, char **argv) {
    map<string, double> args = get_args();

    if (argc == 1) {
        print_args(args);
        goto hopa;
    }

    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0 ||
                      strcmp(argv[1], "-h") == 0))
        return puts(HELP.c_str());

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == 45 && i + 1 < argc) {
            args[argv[i]] = atof(argv[i + 1]);
            if (args[argv[i]] < 0.1 && (strcmp("--brightness", argv[i]) == 0 || strcmp("--gamma", argv[i]) == 0))
                args[argv[i]] = 0.1;
            i++;
        }
    };

    save_config(args);
hopa:
    save_shader(args);
    reload_hyprland_fast();
    return 0;
}
