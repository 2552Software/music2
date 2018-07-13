#include "ofApp.h"
//https://commons.wikimedia.org/wiki/File:Beer_in_glass_close_up.jpg
// This is the most basic pdsp example
// we set up everything and check that everything is working
// before running this also check that the basic oF audio output example is working
// ofxx_folder/examples/sound/audioOutputExample/

// for documentation of the modules and functions:
// http://npisanti.com/ofxPDSP/md__modules.html

/* roate 
ofPushMatrix();
ofTranslate(leafImg.width/2, leafImg.height/2, 0);//move pivot to centre
ofRotate(ofGetFrameNum() * .01, 0, 0, 1);//rotate from centre
ofPushMatrix();
ofTranslate(-leafImg.width/2,-leafImg.height/2,0);//move back by the centre offset
leafImg.draw(0,0);
ofPopMatrix();
ofPopMatrix();
https://www.vidsplay.com/
*/
int AudioPlayer::number = 0;
void AudioPlayer::patch() {

    addModuleOutput("0", fader0);
    addModuleOutput("1", fader1);

    pitchControl >> sampler0.in_pitch();
    pitchControl >> sampler1.in_pitch();

    sampleTrig >> sampler0 >> amp0;
    envGate >> env >> amp0.in_mod();
    sampleTrig >> sampler1 >> amp1;
    env >> amp1.in_mod();

    sampler0 >> amp0 >> fader0;
    sampler1 >> amp1 >> fader1;

    faderControl >> dBtoLin >> fader0.in_mod();
    dBtoLin >> fader1.in_mod();

    sampler0.addSample(&sample, 0);
    sampler1.addSample(&sample, 1);

    smoothControl >> env.in_attack();
    smoothControl >> env.in_release();

    ui.setName("pdsp player " + ofToString(++number));
    ui.add(faderControl.set("volume", 0, -48, 24));
    ui.add(loadButton.set("load", false));
    ui.add(sampleName.set("sample", "no sample"));
    ui.add(samplePath.set("path", "no path"));
    ui.add(pitchControl.set("pitch", 0, -24, 24));
    ui.add(smoothControl.set("fade ms", 0, 0, 50));
    ui.add(bPlay.set("play", false));
    ui.add(bPause.set("pause", false));
    ui.add(bStop.set("stop", true));

    loadButton.addListener(this, &AudioPlayer::loadButtonCall);
    samplePath.addListener(this, &AudioPlayer::sampleChangedCall);
    bPlay.addListener(this, &AudioPlayer::onPlay);
    bPause.addListener(this, &AudioPlayer::onPause);
    bStop.addListener(this, &AudioPlayer::onStop);

    bSemaphore = true;
    sample.setVerbose(true);
}

void AudioPlayer::onPlay(bool & value) {

    if (bSemaphore) {
        bSemaphore = false;
        if (bStop) {
            bPlay = true;
            bStop = false;
            envGate.trigger(1.0f);
            sampleTrig.trigger(1.0f);
            ofLogVerbose() << "[pdsp] player: playing\n";
        }
        else if (bPause) {
            ofLogVerbose() << "[pdsp] player: unpaused\n";
            bPlay = true;
            bPause = false;
            envGate.trigger(1.0f);
        }
        else {
            bPlay = true;
            sampleTrig.trigger(1.0f);
        }
        bSemaphore = true;
    }

}

void AudioPlayer::onPause(bool & value) {

    if (bSemaphore) {
        bSemaphore = false;
        if (bPlay) {
            bPause = true;
            bPlay = false;
            ofLogVerbose() << "[pdsp] player: paused\n";
            envGate.off();
        }
        else if (bStop) {
            bPause = false;
            ofLogVerbose() << "[pdsp] player: impossible to pause on stop";
        }
        else {
            ofLogVerbose() << "[pdsp] player: unpaused\n";
            bPlay = true;
            bPause = false;
            envGate.trigger(1.0f);
        }
        bSemaphore = true;
    }
}

void AudioPlayer::onStop(bool & value) {

    if (bSemaphore) {
        bSemaphore = false;
        if (bPlay || bPause) {
            bStop = true;
            bPlay = false;
            bPause = false;
            ofLogVerbose() << "[pdsp] player: stopped\n";
            envGate.off();
        }
        bSemaphore = true;
    }
}

void AudioPlayer::loadButtonCall(bool & value) {
    if (value) {

        float fvalue = faderControl.get();
        faderControl.setv(0.0f);

        //Open the Open File Dialog
        ofFileDialogResult openFileResult = ofSystemLoadDialog("select an audio sample");

        //Check if the user opened a file
        if (openFileResult.bSuccess) {

            string path = openFileResult.getPath();

            samplePath = path;

            ofLogVerbose("file loaded");

        }
        else {
            ofLogVerbose("User hit cancel");
        }

        // switch to mono if the sample has just one channel
        if (sample.channels == 1) {
            sampler1.setSample(&sample, 0, 0);
        }
        else {
            sampler1.setSample(&sample, 0, 1);
        }

        loadButton = false;

        faderControl.setv(fvalue);

        bool dummy = true;
        onStop(dummy);
    }
}

void AudioPlayer::sampleChangedCall(string & value) {

    ofLogVerbose("loading" + value);
    loadSample(samplePath);

    auto v = ofSplitString(samplePath, "/");
    sampleName = v[v.size() - 1];
}

void AudioPlayer::loadSample(string path) {
    sample.load(path);
}

void AudioPlayer::load(string path) {
    samplePath = path;
}

void AudioPlayer::play() {
    bPlay = bPlay ? false : true;
}

void AudioPlayer::pause() {
    bPause = bPause ? false : true;
}

void AudioPlayer::stop() {
    bStop = bStop ? false : true;
}
//--------------------------------------------------------------
void ofApp::setup() {
    player.load(ofToDataPath("song3.wav"));
    player.play();
    //-------------------GRAPHIC SETUP--------------
    ofBackground(0);
    ofSetFrameRate(30);
    videos.add("Beer_Pour_Videvo.mp4", ofColor(255, 255, 255), 32.0f, 52.0f);
    videos.add("lighthouse.mp4", ofColor(255, 255, 255));
    videos.setNext();

    background1.load("beer1.jpg");
    beer.load("beer2.jpg");
    popcorn.load("popcorn.jpg");
    burger.load("burger.jpg");
    ignore = 0;
    //--------PATCHING-------

    // a pdsp::ADSR is an ADSR envelope that makes a one-shot modulation when triggered
    // pdsp::ADSR require an output sending trigger signals
    // remember, in pdsp out_trig() always have to be connected to in_trig()
    // in_trig() is the default pdsp::ADSR input signal

    // a pdsp::Amp multiply in_signal() and in_mod()      
    player.out("0") >> engine.audio_out(0);
    player.out("1") >> engine.audio_out(1);
    gate_ctrl.out_trig() >> adsrEnvelop;
    adsrEnvelop >> amp.in_mod();

    pitch_ctrl >> oscillator.in_pitch();
    //oscillator >> amp * dB(-12.0f) >> engine.audio_out(0);
    //amp * dB(-12.0f) >> engine.audio_out(1);
    // we patch the pdsp::Parameter to control pitch and amp
    // and then patch the oscillator to the engine outs
    osc1_pitch_ctrl >> fm1.in_pitch();
    osc2_pitch_ctrl >> fm2.in_pitch();
    osc3_pitch_ctrl >> fm3.in_pitch();


    // pdsp::ParameterGain can be added to gui like an ofParameter
    // and has an input and output for signals
    // it is used to control volume as it has control in deciBel
    // pdsp::ParameterAmp instead just multiply the input for the parameter value
    // it is usefule for scaling modulation signals 
    fm1 >> osc1_amp >> engine.audio_out(0);
    osc1_amp >> engine.audio_out(1);
    //fm2 >> osc2_gain >> engine.audio_out(0);
   // osc2_gain >> engine.audio_out(1);
    //fm3 >> osc3_gain >> engine.audio_out(0);
   // osc3_gain >> engine.audio_out(1);
    osc1_pitch_ctrl.set("pitch", 60.0f, 24.0f, 96.0f);
    osc1_amp.set("amp", 0.25f, 0.0f, 1.0f);
    osc2_pitch_ctrl.set("pitch", 60, 24, 96);
    osc2_gain.set("active", false, -48.0f, -12.0f);
    osc3_pitch_ctrl.set("pitch coarse", 60, 24, 96);
    osc3_pitch_ctrl.set("pitch fine  ", 0.0f, -0.5f, 0.5f);
    osc3_gain.set("gain", -24.f, -48.0f, 0.0f);
    osc2_pitch_ctrl.setv(ofRandom(48.0f, 96.0f));

    1.0f >> adsrEnvelop.in_attack();
    50.0f >> adsrEnvelop.in_decay();
    0.5f >> adsrEnvelop.in_sustain();
    500.0f >> adsrEnvelop.in_release();
    gate_ctrl.trigger(1.0f);

    pitch_ctrl.setv(36.0f); // we control the value of an pdsp::Parameter directly with the setv function

                            // you can smooth out an pdsp::Parameter changes, decomment this for less "grainy" pitch changes
    pitch_ctrl.enableSmoothing(50.0f); // 50ms smoothing
                                       //----------------------AUDIO SETUP-------------

    // set up the audio output device
    engine.listDevices();
    engine.setDeviceID(0); // REMEMBER TO SET THIS AT THE RIGHT INDEX!!!!

                           // start your audio engine !
    engine.setup(44100, 512, 3);
    // arguments are : sample rate, buffer size, and how many buffer there are in the audio callback queue
    // 512 is the minimum buffer size for the raspberry Pi to work
    // 3 buffers queue is the minimum for the rPi to work
    // if you are using JACK you have to set this number to the bufferSize you set in JACK
    // on Windows you have to set the sample rate to the system sample rate, usually 44100hz
    // on iOS sometimes the systems forces the sample rate to 48khz, so if you have problems set 48000

}

//--------------------------------------------------------------
void ofApp::update() {
    videos.update();
    pitch_ctrl.setv(videos.getPitch());
    osc1_pitch_ctrl.setv(videos.getPitch());
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofPushMatrix();
    if (!ignore) {
        background1.draw(0, 0, ofGetScreenWidth(), ofGetScreenHeight());
    }
    ofEnableAlphaBlending();
    ofSetColor(videos.getColor().r, videos.getColor().g, videos.getColor().b, videos.getAlpha());
    videos.draw(0, 0, ofGetScreenWidth(), ofGetScreenHeight());
    ofDisableAlphaBlending();
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    float pitch = ofMap(x, 0, ofGetWidth(), 36.0f, 72.0f);
    pitch_ctrl.setv(pitch);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    float pitch = ofMap(x, 0, ofGetWidth(), 36.0f, 72.0f);
    pitch_ctrl.setv(pitch);

    // y value controls the trigger intensity
    float trig = ofMap(y, 0, ofGetHeight(), 1.0f, 0.000001f);
    gate_ctrl.trigger(trig); // we send a trigger to the envelope
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    gate_ctrl.off(); // we send an "off" trigger to the envelope
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
