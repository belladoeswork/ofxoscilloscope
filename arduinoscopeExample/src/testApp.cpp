#include "testApp.h"

//
//  arduinoscope
//
//  openFrameworks program to display arduino data as on an oscilloscope
//  
//  Requires:
//	ofxOscilloscope addon
//	StandardFirmata Example running on the Arduino
//
//  Created by Sean Montgomery on 3/31/13.
//
//  This work is licensed under the Creative Commons 
//  Attribution-ShareAlike 3.0 Unported License. 
//  To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
//

//--------------------------------------------------------------
void testApp::setup(){
	ofBackground(255,255,255);
	ofTrueTypeFont legendFont;
	legendFont.loadFont("verdana.ttf", 12, true, true);

	// pins
	// timeWindow
	// yScale
	// yOffset
	// colors
	// names

	// Setup Arduino
	string arduinoPort = "\\\\.\\COM10";
	arduino.connect(arduinoPort, 57600);
	while(!arduino.isArduinoReady()); 
	arduino.sendAnalogPinReporting(0, ARD_ANALOG);
	arduino.sendAnalogPinReporting(1, ARD_ANALOG);
	arduino.sendAnalogPinReporting(2, ARD_ANALOG);
	arduino.sendAnalogPinReporting(3, ARD_ANALOG);
	arduino.sendAnalogPinReporting(4, ARD_ANALOG);
	arduino.update();

	// Log Directory
	logDirPath = "../LogData/";
	logger.setDirPath(logDirPath);

	nScopes = 2;
	ofRectangle scopeArea = ofRectangle(ofPoint(0,0), ofGetWindowSize());
	scopeWin = ofxMultiScope(nScopes, scopeArea, legendFont); // Setup the multiScope panel

	samplingFreq = 50.; // Sampling rate (Hz)
	{ // Scope 1 setup
		const int nVariables = 3;
		nPlotsScope0 = nVariables;
		float timeWindow = 5.; // seconds
		float yScale = 0.5; // yScale multiplier
		float yOffset = -256.; // yOffset from the center of the scope window
		ofColor colors[nVariables] = {ofColor(255,0,0), ofColor(0,255,0), ofColor(0,0,255)};
		std::vector<ofColor> vec_colors(colors, colors + nVariables);

		string names[nVariables] = {"A0", "A1", "A2"};
		std::vector<string> vec_names(names, names + nVariables);
		scopeWin.scopes.at(0).setup(timeWindow, samplingFreq, vec_names, vec_colors, 
			yScale, yOffset); // Setup each oscilloscope panel
	}
	{ // Scope 2 setup
		const int nVariables = 2;
		nPlotsScope1 = nVariables;
		float timeWindow = 10.; // seconds
		float yScale = 0.5; // yScale multiplier
		float yOffset = -256.; // yOffset from the center of the scope window
		ofColor colors[nVariables] = {ofColor(0,0,0), ofColor(255,0,255)};
		std::vector<ofColor> vec_colors(colors, colors + nVariables);

		string names[nVariables] = {"A3", "A4"};
		std::vector<string> vec_names(names, names + nVariables);
		scopeWin.scopes.at(1).setup(timeWindow, samplingFreq, vec_names, vec_colors, 
			yScale, yOffset); // Setup each oscilloscope panel
	}

	selectedScope = 0; // Select all scopes for increment/decrement

	isPaused = false;
	isRecording = false;
	loopTimer = (unsigned long) ofGetElapsedTimeMillis();

	logger.startThread();
}

//--------------------------------------------------------------
void testApp::update(){
	unsigned long now = (unsigned long) ofGetElapsedTimeMillis();
	unsigned long loopDuration = (now - loopTimer);
	if (loopDuration >= (1000/samplingFreq) ) {
		loopTimer = now;
		cout << "loop duration: " << loopDuration << "\n";

		arduino.update();

		std::stringstream logData;
		logData << now << ",";

		// Scope 1 - Analog Data
		vector<float> analogData1;
		analogData1.resize(nPlotsScope0);
		for (int i=0; i<nPlotsScope0; i++) {
			analogData1.at(i) = (arduino.getAnalog(i));
			logData << analogData1.at(i) << ",";
		}
		if (!isPaused) {
			scopeWin.scopes.at(0).updateData(analogData1);
		}

		// Scope 2 - Analog Data
		vector<float> analogData2;
		analogData2.resize(nPlotsScope1);
		for (int i=0; i<nPlotsScope1; i++) {
			analogData2.at(i) = (arduino.getAnalog(i+nPlotsScope0));
			logData << analogData2.at(i) << ",";
		}

		if (!isPaused) {
			scopeWin.scopes.at(1).updateData(analogData2);
		}

		if (isRecording) {
			logData << "\n";
			if (logger.isThreadRunning()) { // Threaded logging to improve speed
				logger.lock();
				logger.loggerQueue.push(logData.str());
				logger.unlock();
			} else {
				// Unthreaded logging
				logger.log(logData.str());
			}

		}
	}

}

//--------------------------------------------------------------
void testApp::draw(){
	if (isRecording) {
		// Show "RECORDING" on panel
		ofTrueTypeFont font;
		font.loadFont("verdana.ttf", 12, true, true);
		ofSetColor(255, 0, 0);
		ofRectangle panelRect = scopeWin.getPosition();
		ofPoint br = panelRect.getBottomRight();
		string text = "RECORDING";
		ofRectangle textBox = font.getStringBoundingBox(text,0,0);
		float pad = 10.;
		font.drawString(text, br.x-pad-textBox.width, br.y-pad-textBox.height);
	}
	scopeWin.plot();
}

//--------------------------------------------------------------
void testApp::exit(){
	printf("exit()");
	logger.waitForThread(true);

	arduino.disconnect();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
	// Increment the yScale
	if (key == '+') {
		if (selectedScope == 0) {
			scopeWin.incrementYScale();
		} else {
			scopeWin.scopes.at(selectedScope - 1).incrementYScale();
		}
	}
	
	// Decrement the yScale
	if ((key == '-') || (key == '_')) {
		if (selectedScope == 0) {
			scopeWin.decrementYScale();
		} else {
			scopeWin.scopes.at(selectedScope - 1).decrementYScale();
		}
	}

	// Increment the yOffset
	if (key == 357) { // Up Arrow
		if (selectedScope == 0) {
			scopeWin.incrementYOffset();
		} else {
			scopeWin.scopes.at(selectedScope - 1).incrementYOffset();
		}
	}
	
	// Decrement the yOffset
	if (key == 359) { // Down Arrow
		if (selectedScope == 0) {
			scopeWin.decrementYOffset();
		} else {
			scopeWin.scopes.at(selectedScope - 1).decrementYOffset();
		}
	}

	// Increment the timeWindow
	if (key == 358) { // Right Arrow
		if (selectedScope == 0) {
			scopeWin.incrementTimeWindow();
		} else {
			scopeWin.scopes.at(selectedScope - 1).incrementTimeWindow();
		}
	}
	
	// Decrement the timeWindow
	if (key == 356) { // Left Arrow
		if (selectedScope == 0) {
			scopeWin.decrementTimeWindow();
		} else {
			scopeWin.scopes.at(selectedScope - 1).decrementTimeWindow();
		}
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	cout << "Key Released: " << key << "\n";

	// Choose an oscilloscope panel for changing yScale/yOffset/timeWindow
	// Starts counting from 1
	// Zero = all
	if ((key >= 48) && (key <= 57)){
		int number = key - 48;
		if (number <= nScopes) {
			selectedScope = number;
			scopeWin.setOutlineWidth(1);
			if (selectedScope > 0) {
				scopeWin.scopes.at(selectedScope - 1).setOutlineWidth(5);
			}
		}
	}

	if (key == 'r') {
		isRecording = !isRecording;
	}

	if (key == 32) { // Space Bar
		isPaused = !isPaused;
	}


	
	/*
	// testing yScale
	if (key == '1') {
		scopeWin.scopes.at(1).setYScale(scopeWin.scopes.at(1).getYScale() / 2); 
	}
	if (key == '2') {
		scopeWin.scopes.at(1).setYScale(scopeWin.scopes.at(1).getYScale() * 2); 
	}

	// testing yOffset
	if (key == '3') {
		scopeWin.scopes.at(1).setYOffset(scopeWin.scopes.at(1).getYOffset() - 10); 
	}
	if (key == '4') {
		scopeWin.scopes.at(1).setYOffset(scopeWin.scopes.at(1).getYOffset() + 10); 
	}
	*/

/*
	// testing setVariableNames
	if (key == 'n') {
		const int nVariables = 2;
		string names[nVariables] = {"happy", "joy"};
		std::vector<string> vec_names(names, names + nVariables);
		scopeWin.scopes.at(1).setVariableNames(vec_names); 
	}
	if (key == 'm') {
		const int nVariables = 2;
		string names[nVariables] = {"wow", "zing"};
		std::vector<string> vec_names(names, names + nVariables);
		scopeWin.scopes.at(1).setVariableNames(vec_names); 
	}

	//testing setVariableColors
	if (key == 'z') {
		const int nVariables = 2;
		ofColor colors[nVariables] = {ofColor(0,0,0), ofColor(255,0,0)};
		std::vector<ofColor> vec_colors(colors, colors + nVariables);
		scopeWin.scopes.at(1).setVariableColors(vec_colors); 
	}
	if (key == 'x') {
		const int nVariables = 2;
		ofColor colors[nVariables] = {ofColor(0,255,0), ofColor(255,255,0)};
		std::vector<ofColor> vec_colors(colors, colors + nVariables);
		scopeWin.scopes.at(1).setVariableColors(vec_colors); 
	}

	//testing setTimeWindow
	if (key == 't') {
		scopeWin.setTimeWindow(5);
	}
	if (key == 'y') {
		scopeWin.setTimeWindow(10);
	}
	if (key == 'u') {
		scopeWin.scopes.at(1).setTimeWindow(15);
	}

	//testing setPosition
	if (key == 'a') {
		ofRectangle pos = scopeWin.scopes.at(1).getPosition();
		pos.setX(pos.getX() + 50);
		scopeWin.scopes.at(1).setPosition(pos);
	}
	if (key == 's') {
		ofRectangle pos = scopeWin.getPosition();
		pos.setX(pos.getX() + 50);
		scopeWin.setPosition(pos);
	}
	if (key == 'd') {
		ofPoint min = ofPoint(0., 0.);
		ofPoint max = ofPoint(ofGetWindowSize().x, ofGetWindowSize().y);
		scopeWin.setPosition(min, max);
	}
	if (key == 'f') {
		ofPoint min = ofPoint(50., 300.);
		ofPoint max = ofPoint(ofGetWindowSize().x - 200., ofGetWindowSize().y - 200.);
		scopeWin.setPosition(min, max);
	}

	// testing backgroundColor
	if (key == 'c') {
		ofColor color = ofColor(0,0,0,0.);
		scopeWin.setBackgroundColor(color);
	}
	if (key == 'v') {
		ofColor color = ofColor(0,255,0,128);
		scopeWin.setBackgroundColor(color);
	}
	if (key == 'b') {
		ofColor color = ofColor(0,0,255,128);
		scopeWin.scopes.at(1).setBackgroundColor(color);
	}

	// testing setOutlineColor
	if (key == 'j') {
		ofColor color = ofColor(0,0,0,255);
		scopeWin.setOutlineColor(color);
	}
	if (key == 'k') {
		ofColor color = ofColor(0,255,0,128);
		scopeWin.setOutlineColor(color);
	}
	if (key == 'l') {
		ofColor color = ofColor(0,0,255,128);
		scopeWin.scopes.at(1).setOutlineColor(color);
	}

	// testing setZeroLineColor
	if (key == 'i') {
		ofColor color = ofColor(0,0,0,255);
		scopeWin.setZeroLineColor(color);
	}
	if (key == 'o') {
		ofColor color = ofColor(0,255,0,128);
		scopeWin.setZeroLineColor(color);
	}
	if (key == 'p') {
		ofColor color = ofColor(0,0,255,128);
		scopeWin.scopes.at(1).setZeroLineColor(color);
	}

	// testing setLegendFont
	if (key == 'g') {
		ofTrueTypeFont legendFont;
		legendFont.loadFont("verdana.ttf", 12, true, true);
		legendFont.setLetterSpacing(0.7);
		scopeWin.setLegendFont(legendFont);
	}
	if (key == 'h') {
		ofTrueTypeFont legendFont;
		legendFont.loadFont("verdana.ttf", 12, true, true);
		legendFont.setLineHeight(15.0f);
		legendFont.setLetterSpacing(1.5);
		scopeWin.scopes.at(1).setLegendFont(legendFont);
	}

	// testing setLegendWidth
	if (key == 'q') {
		scopeWin.setLegendWidth(75);
	}
	if (key == 'w') {
		scopeWin.scopes.at(1).setLegendWidth(200);
	}

	// testing setTextSpacing
	if (key == 'e') {
		scopeWin.setTextSpacing(15, 15);
	}
	if (key == 'r') {
		scopeWin.scopes.at(1).setTextSpacing(20, 10);
	}
	*/
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}