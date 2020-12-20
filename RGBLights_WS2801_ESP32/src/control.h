enum ControlMode {whitelight, animations, undefined};

constexpr const char * ControlModeStrings[] = {"WhiteLight", "Animation"};

constexpr const char * str(const ControlMode & cm) {
    return ControlModeStrings[cm];
}

ControlMode next(const ControlMode & cm) {
    int count = (sizeof(ControlModeStrings) / sizeof(ControlModeStrings[0]));
    int idx = cm;
    int nxt = (idx + 1) % count;
    return static_cast<ControlMode>(nxt);
}