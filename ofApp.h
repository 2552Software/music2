#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
class AudioPlayer : public pdsp::Patchable {

public:
    AudioPlayer() { patch(); }
    AudioPlayer(const AudioPlayer & other) { patch(); }

    ofParameterGroup    ui;

    void load(string path);
    void play();
    void pause();
    void stop();

private:
    void patch();

    void loadButtonCall(bool & value);
    void sampleChangedCall(string & value);
    void loadSample(string path);

    pdsp::Sampler       sampler0;
    pdsp::Sampler       sampler1;
    pdsp::ADSR          env;
    pdsp::Amp           amp0;
    pdsp::Amp           amp1;

    pdsp::Amp           fader0;
    pdsp::Amp           fader1;
    pdsp::DBtoLin       dBtoLin;
    pdsp::Parameter     faderControl;

    pdsp::Parameter     pitchControl;
    pdsp::Parameter     smoothControl;

    pdsp::SampleBuffer  sample;
    ofParameter<bool>   loadButton;
    ofParameter<string> samplePath;
    ofParameter<string> sampleName;

    ofParameter<bool>   bPlay;
    ofParameter<bool>   bPause;
    ofParameter<bool>   bStop;

    pdsp::TriggerControl    sampleTrig;
    pdsp::TriggerControl    envGate;

    static int number;

    void onPlay(bool & value);
    void onPause(bool & value);
    void onStop(bool & value);

    bool bSemaphore;
};
class videoPlayer {
public:
    float low=32.0, high=72.0;
    ofColor color;

    void add(std::string path, const ofColor& color,float low = 32.0, float high = 72.0, ofLoopType state = ofLoopType::OF_LOOP_NORMAL) {
        this->low = low;
        this->high = high;
        this->color = color;
        ofVideoPlayer player;
        player.load(path);
        videos.push_back(player);
        current = videos.size() - 1;
        videos[current].setLoopState(state);
    }
    unsigned getAlpha() {
        return ofMap(getCurrentFrame(), 0, getTotalNumFrames(), 20, 255);
    }
    ofColor& getColor() {
        return color;
    }
    float getPitch() {
        return ofMap(getCurrentFrame(), 0, getTotalNumFrames(), low, high);
    }
    void setCurrent(int pos=0) {
        if (pos >= 0 && pos < videos.size()) {
            current = pos;
        }
    }
    void setNext() {
        videos[current].stop();
        if (++current >= videos.size()) {
            current = 0;
        }
        play();
    }
    int getTotalNumFrames() const {
        return videos[current].getTotalNumFrames();
    }
    int getCurrentFrame() const {
        return videos[current].getCurrentFrame();
    }
    // plays the current video, if any
    void play() {
        videos[current].play();
    }
    void update() {
        if (videos[current].getCurrentFrame() >= videos[current].getTotalNumFrames()) {
            setNext();
        }
        videos[current].update();
    }
    void draw(int x, int y, int cx, int cy) {
        videos[current].draw(x, y, cx, cy);
    }
private:
    int current = -1;
    std::vector<ofVideoPlayer> videos;
};

class ofApp : public ofBaseApp {

public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // pdsp modules
    pdsp::Engine            engine;
    pdsp::VAOscillator      oscillator;
    pdsp::LFO               lfo;
    pdsp::Amp               amp;
    pdsp::ADSR              adsrEnvelop; //Attack-Decay-Sustain-Release envelope.
    pdsp::TriggerControl    gate_ctrl;
    pdsp::Parameter         pitch_ctrl;
    pdsp::FMOperator        fm1;
    pdsp::FMOperator        fm2;
    pdsp::FMOperator        fm3;
    pdsp::Parameter      osc1_pitch_ctrl;
    pdsp::Parameter      osc2_pitch_ctrl;
    pdsp::Parameter      osc3_pitch_ctrl;
    pdsp::ParameterAmp   osc1_amp;
    pdsp::ParameterGain  osc2_gain;
    pdsp::ParameterGain  osc3_gain;

    // toys
    AudioPlayer     player;
    ofImage background1;
    ofImage beer;
    ofImage popcorn;
    ofImage burger;
    bool ignore;

    // data
    videoPlayer videos;
};
