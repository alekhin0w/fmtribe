#include "seq.h"
#include "string.h"
#include "assert.h"

#define METRONOME_CH    CHANNELS

seq_t seq_new() {
    seq_t new = {
        .instrs = {},
        .seq = {},
        .mseq = {},
        .muted_channels = {},

        .current_uclocks_per_step = 0,
        .current_bpm = 0,
        .current_frame = 0,
        .current_step = 0,

        .prev_tap = NULL,

        .pause_after_current_step = false,
        .stop_after_pattern_ends = false,
        .playing = false,
        .metronome_on = false,

        .current_selected_channel = 0,
        .current_selected_frame = 0,
        .follow = true,
        .apply_all_frames = true,
        .play_instruments = false,
        .recording = false,

        .dirty = true,
    };
    return new;
};

void seq_tick(seq_t* this)
{
    this->current_step++;

    if (this->current_step == STEPS) {
        this->current_step = 0;
        this->current_frame++;
    }

    if (this->current_frame == FRAMES) {
        this->current_frame = 0;
        if (this->stop_after_pattern_ends) {
            this->playing = false;
            this->stop_after_pattern_ends = false;
        }
    }

    if (this->follow) {
        this->current_selected_frame = this->current_frame;
    }

    this->dirty = true;
}

void seq_play_channel(const seq_t* this, const unsigned int c)
{
    fm_key_off(c);
    fm_key_on(c, this->instrs[c].octave, this->instrs[c].note);
}

void seq_play_step(const seq_t* this)
{
    if (this->metronome_on) {
        if (this->current_step == 0) {
            fm_key_off(METRONOME_CH);
            fm_key_on(METRONOME_CH, 4, E);
        } else if (this->current_step % 4 == 0) {
            fm_key_off(METRONOME_CH);
            fm_key_on(METRONOME_CH, 3, E);
        }
    }

    for (int c = 0; c < CHANNELS; c++) {
        if (!this->muted_channels[c] && this->seq[c][this->current_frame][this->current_step]) {
            seq_play_channel(this, c);
        }
    }
}

void seq_set_bpm(seq_t* this, const unsigned int value)
{
    assert(value > 0);
    this->current_bpm = value;
    this->current_uclocks_per_step = (UCLOCKS_PER_MIN / value) / 4;
}

void seq_set_bpm_from_uclocks_per_beat(seq_t* this, const uclock_t uclocks)
{
    assert(uclocks > 0);
    this->current_bpm = UCLOCKS_PER_MIN / uclocks;
    this->current_uclocks_per_step = uclocks / 4;
}

void seq_clear_seq(seq_t* this, const int channel)
{
    if (this->apply_all_frames) {
        memset(this->seq[channel], 0, FRAMES * STEPS * sizeof(bool));
        memset(this->mseq[channel], 0, FRAMES * STEPS * sizeof(unsigned int));
    } else {
        memset(this->seq[channel][this->current_selected_frame], 0, STEPS * sizeof(bool));
        memset(this->mseq[channel][this->current_selected_frame], 0, STEPS * sizeof(unsigned int));
    }
    this->dirty = true;
}

void seq_clear_seq_all(seq_t* this)
{
    if (this->apply_all_frames) {
        memset(this->seq, 0, CHANNELS * FRAMES * STEPS * sizeof(bool));
        memset(this->mseq, 0, CHANNELS * FRAMES * STEPS * sizeof(unsigned int));
    } else {
        for (int c = 0; c < CHANNELS; c++) {
            memset(this->seq[c][this->current_selected_frame], 0, STEPS * sizeof(bool));
            memset(this->mseq[c][this->current_selected_frame], 0, STEPS * sizeof(unsigned int));
        }
    }
    this->dirty = true;
}

void seq_select_prev_channel(seq_t* this)
{
    if (this->current_selected_channel == 0) {
        this->current_selected_channel = CHANNELS - 1;
    } else {
        this->current_selected_channel--;
    }
    this->dirty = true;
}

void seq_select_next_channel(seq_t* this)
{
    if (this->current_selected_channel == CHANNELS - 1) {
        this->current_selected_channel = 0;
    } else {
        this->current_selected_channel++;
    }
    this->dirty = true;
}

void seq_select_prev_frame(seq_t* this)
{
    if (this->current_selected_frame == 0) {
        this->current_selected_frame = FRAMES - 1;
    } else {
        this->current_selected_frame--;
    }
    this->follow = false;
    this->dirty = true;
}

void seq_select_next_frame(seq_t* this)
{
    if (this->current_selected_frame == FRAMES - 1) {
        this->current_selected_frame = 0;
    } else {
        this->current_selected_frame++;
    }
    this->follow = false;
    this->dirty = true;
}

void seq_toggle_metronome(seq_t* this)
{
    this->metronome_on = Not(this->metronome_on);
}

void seq_toggle_follow(seq_t* this)
{
    this->follow = Not(this->follow);
    this->dirty = true;
}

void seq_toggle_apply_all_frames(seq_t* this)
{
    this->apply_all_frames = Not(this->apply_all_frames);
    this->dirty = true;
}

void seq_toggle_play_instruments(seq_t* this)
{
    this->play_instruments = Not(this->play_instruments);
    this->dirty = true;
}

void seq_toggle_recording(seq_t* this)
{
    this->recording = Not(this->recording);
    this->dirty = true;
}

void seq_tap_tempo(seq_t* this)
{
    uclock_t now = uclock();
    if (this->prev_tap) {
        seq_set_bpm_from_uclocks_per_beat(this, now - this->prev_tap);
    }
    this->prev_tap = now;
}
