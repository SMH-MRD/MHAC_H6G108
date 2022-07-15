#pragma once

class CPushButton
{
public:
    CPushButton(){}
    ~CPushButton() {}

    void set_on_hold_time(int count) { off_delay_time = count; return; };
    void set_on() {
        triggered = true;
        hold_time_count = 0;
        return;
    }
    void action() {
        if (triggered)hold_time_count++;
        if (hold_time_count > off_delay_time) {
            triggered = false; hold_time_count = 0;
        }
        return;
    }
    bool is_pb_on() { return(triggered); }

private:
    bool triggered = true;
    int hold_time_count = 0;
    int off_delay_time = 4;

};

