#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){
	
	vidGrabber.initGrabber(W,H);
	
	ofSetWindowShape(WINDOW_WIDTH, WINDOW_HEIGHT);
	ofSetFrameRate(120);
	ofSetBackgroundAuto(true);
	ofBackground(0,0,0);
	
	
	orientationTracker.setListener(this);
	
	currentSound = 0;
	numSounds = 6;
	
	string samples[] = {"breathe.wav", "street-footsteps.wav", 
		"tile-footsteps.wav", "mosquito1.wav",
		"mosquito2.wav", "telephone.wav"};
	
	for (int i = 0; i < numSounds; i++) {
		sound[i].loadSound(samples[i], true);
		sound[i].setMultiPlay(true);
		sound[i].setLoop(true);
	}
	
}
//--------------------------------------------------------------
void testApp::update(){
	
	ofBackground(0,0,0);
	
	vidGrabber.update();
	if (vidGrabber.isFrameNew()) {
		orientationTracker.update(vidGrabber.getPixels(), W, H);

	}
		
}
	
//--------------------------------------------------------------
void testApp::draw(){
	orientationTracker.draw(0, 0);
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	switch (key){
		case 's':
			vidGrabber.videoSettings();
			break;
		case 'f':
			ofToggleFullscreen();
			break;
		default:
			orientationTracker.keyPressed(key);
			break;
	}
	
}

void testApp::blobOn( int x, int y, int id, int order )
{
	printf("blob on\n");
	
	// start a sound player with speed 0
	sound[currentSound].play();
	sound[currentSound].setSpeed(0.0);
	
	// keep our mappings
	soundMapping[id] = currentSound;
	velocityMapping[id] = velocities.size();
	
	velocities.push_back(0);
	px.push_back(x);
	py.push_back(y);
	
	currentSound = (currentSound + 1) % numSounds;
}
void testApp::blobMoved( int x, int y, int id, int order )
{
	printf("blob moved\n");	
	int previous_x = px[velocityMapping[id]];
	int previous_y = py[velocityMapping[id]];
	
	float speed = sqrtf( (x - previous_x)*(x - previous_x) + 
 					   (y - previous_y)*(y - previous_y) ) / 20.0f;
	
	px[velocityMapping[id]] = x;
	py[velocityMapping[id]] = y;
	
	velocities[velocityMapping[id]] = 0.9 * velocities[velocityMapping[id]] + 0.1 * speed;
	
	printf("%f\n", velocities[velocityMapping[id]]);
	
	sound[soundMapping[id]].setSpeed(velocities[velocityMapping[id]]);
}
void testApp::blobOff( int x, int y, int id, int order )
{
	printf("blob off\n");
	sound[soundMapping[id]].setSpeed(0);
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
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


