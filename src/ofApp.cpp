#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {

	ofSetLogLevel(OF_LOG_VERBOSE);

	// repatable randomness
	ofSeedRandom(10);



	// simulation specific stuff goes here

	string gameStateLabels[] = { "START", "PLAY", "FIRED", "HIT" };
	gameStates.assign(gameStateLabels, gameStateLabels + 4);
	gameState = PLAY;




	gui.setup();
	ImGui::GetIO().MouseDrawCursor = false;

	// load parameters from file
	// loadFromFile("settings.xml");

	// instantiate the ground
	ground.set(RANGE, RANGE);
	ground.rotate(90, 1, 0, 0);

	// lift camera to 'eye' level
	easyCam.setDistance(RANGE);
	float d = easyCam.getDistance();
	easyCam.setPosition(ofVec3f(0, cameraHeightRatio*d, d*sqrt(1.0f - cameraHeightRatio*cameraHeightRatio)) + easyCamTarget);
	easyCam.setTarget(easyCamTarget);

	// simulation specific stuff goes here
	muzzleSpeed = 3.0f;		//changed muzzleSpeed to limit the total distance possible
	reset();
	balls.reserve(50);		//reserves space for the balls that will be created.


	for (int i = 0; i < 50; i++) {
		YAMPE::Particle::Ref ballParticle(new YAMPE::Particle());
		float radius = (50 - i) * 0.001;
		ballParticle->setRadius(radius);
		balls.push_back(ballParticle);
	}
}



void ofApp::reset() {

	t = 0.0f;

	//random target position
	target.set(ofRandom(1) * 15.0f - 7.5f, 0, ofRandom(1) * 15.0f - 7.5f);


	// simulation specific stuff goes here
	ball.force = ofVec3f();
	ball.acceleration = ofVec3f();
	ball.velocity = ofVec3f();
	ball.position = ofVec3f();

}

void ofApp::update() {

	float dt = ofClamp(ofGetLastFrameTime(), 0.0, 0.02);
	if (!isRunning) return;
	t += dt;
	

	// simulation specific stuff goes here


	if (dt > 0) {
		if (ball.position.y > 0) {		//if the ball is higher than the ground
			ball.acceleration = ofVec3f(0, -0.981f, 0);		//ball moves as gravity acts upon it
		}
		else {
			//the ball must be below the ground
			if (ball.position.y <= 0) {
				gameState = HIT;	//change the gamestate. Check collision later
				
			}
			ball.velocity = ofVec3f(0, 0, 0);	//reset the balls velocity
			ball.position.y = 0;				//set the balls y position to 0

		}
		for (int i = balls.size() - 2; i >= 0; i--) {		//loop through the ball vector
			balls[i + 1]->position.x = balls[i]->position.x;	//cheating 3d
			balls[i + 1]->position.y = balls[i]->position.y;
			balls[i + 1]->position.z = balls[i]->position.z;
		}

		//assign the current vector ball value to the particle ball 
		balls[0]->position.x = ball.position.x;	
		balls[0]->position.y = ball.position.y;
		balls[0]->position.z = ball.position.z;
		
		ball.integrate(dt);		//update the balls position
	}

}

void ofApp::fire() {

	ball.position = ofVec3f(0, 0.5, 0);
	ball.velocity = ofVec3f(0, 0, 0);
	ball.acceleration = ofVec3f(0, 0, 0);

	float changedDirection = direction + 90.0f;		//need to add 90 to the direction to get correct result when using slider
	float changedElevation = 90.0f - elevation;		//need to minus the elevation from 90 to get correct result when using slider

	//convert from degrees to radians
	
	float XDirection = ((sin(ofDegToRad(changedDirection)) * sin(ofDegToRad(changedElevation))) * muzzleSpeed);		//sin(direction) * sin(elevation) * speed
	float YDirection = (cos(ofDegToRad(changedElevation)) * muzzleSpeed);											//cos(elevation * speed
	float ZDirection = ((cos(ofDegToRad(changedDirection)) * sin(ofDegToRad(changedElevation))) * muzzleSpeed);		//cos(direction) * sin(elevation) * speed
	

	ball.setVelocity(ofVec3f(XDirection, YDirection, ZDirection));		
	gameState = FIRED;
	


}

void ofApp::aim() {

}





void ofApp::draw() {




	ofEnableDepthTest();
	ofBackgroundGradient(ofColor(128), ofColor(0), OF_GRADIENT_BAR);

	ofPushStyle();
	//cout <<"Camera" <<easyCamTarget <<endl;
	easyCam.begin();

	ofDrawGrid(RANGE / (2 * 8), 8, false, isXGridVisible, isYGridVisible, isZGridVisible);

	if (isAxisVisible) ofDrawAxis(1);

	if (isGroundVisible) {
		ofPushStyle();
		ofSetHexColor(0xB87333);
		ground.draw();
		ofPopStyle();
	}

	// simulation specific stuff goes here
	ofSetColor(255, 0, 0);
	ofDrawBox(0, 0.2, 0, 0.8, 0.4, 0.8);
	ofDrawSphere(0, 0.4, 0, 0.3);
	
	ofPushMatrix();
	ofTranslate(0, 0.5, 0);
	float rightDirection = direction + 90.0f;
	ofRotateY(rightDirection);
	float rightElevation = 90.0f - elevation;
	ofRotateX(rightElevation);
	ofTranslate(0, -0.5, 0);
	ofSetColor(255, 128, 0);
	ofDrawCylinder(0, 1.0, 0, 0.15, 1);
	ofPopMatrix();

	//drawt the target
	ofSetColor(0, 255, 120);
	ofDrawBox(target.x, 0, target.z, 1, 0.15, 1);

	//draw the balls
	ofSetColor(255, 120, 65);
	for (int i = 0; i < balls.size(); i++) {
		balls[i]->draw();
	}

	ball.draw();

	ofPopStyle();


	easyCam.end();
	ofPopStyle();

	// draw gui elements
	gui.begin();
	drawAppMenuBar();
	drawMainWindow();
	drawLoggingWindow();


	gui.end();


}


void ofApp::drawAppMenuBar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "")) quit();
			ImGui::EndMenu();
		}

		float d = easyCam.getDistance();

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem(isAxisVisible ? "Hide Unit Axis" : "Show Unit Axis", "")) isAxisVisible = !isAxisVisible;
			if (ImGui::MenuItem(isGroundVisible ? "Hide Ground" : "Show Ground", "")) isGroundVisible = !isGroundVisible;
			if (ImGui::MenuItem(isXGridVisible ? "Hide Grid (X)" : "Show Grid (X)", "")) isXGridVisible = !isXGridVisible;
			if (ImGui::MenuItem(isYGridVisible ? "Hide Grid (Y)" : "Show Grid (Y)", "")) isYGridVisible = !isYGridVisible;
			if (ImGui::MenuItem(isZGridVisible ? "Hide Grid (Z)" : "Show Grid (Z)", "")) isZGridVisible = !isZGridVisible;
			ImGui::Separator();
			if (ImGui::MenuItem("Align camera above X axis ", "")) {
				easyCam.setPosition(ofVec3f(d*sqrt(1.0f - cameraHeightRatio*cameraHeightRatio), cameraHeightRatio*d, 0) + easyCamTarget);
				easyCam.setTarget(easyCamTarget);
			}
			if (ImGui::MenuItem("Align camera above Z axis ", "")) {
				easyCam.setPosition(ofVec3f(0, cameraHeightRatio*d, d*sqrt(1.0f - cameraHeightRatio*cameraHeightRatio)) + easyCamTarget);
				easyCam.setTarget(easyCamTarget);
				cout << "here";
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Align camera along X axis ", "")) {
				easyCam.setPosition(ofVec3f(d, 0, 0) + easyCamTarget);
				easyCam.setTarget(easyCamTarget);
			}
			if (ImGui::MenuItem("Align camera along Y axis ", "")) {
				easyCam.setPosition(ofVec3f(0.001, d, 0) + easyCamTarget);
				easyCam.setTarget(easyCamTarget);
			}
			if (ImGui::MenuItem("Align camera along Z axis ", "")) {
				easyCam.setPosition(ofVec3f(0, 0, d) + easyCamTarget);
				easyCam.setTarget(easyCamTarget);
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}


void ofApp::drawMainWindow() {


	ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiSetCond_FirstUseEver);
	if (ImGui::Begin("Main")) {

		if (ImGui::CollapsingHeader("3D")) {
			if (ImGui::Button("Reset##CameraTarget")) {
				easyCamTarget = ofVec3f(0, 5, 0);
				easyCam.setTarget(easyCamTarget);
			}

			ImGui::SameLine();
			if (ImGui::InputFloat3("Camera Target", &easyCamTarget.x)) {
				easyCam.setTarget(easyCamTarget);
			}
			if (ImGui::SliderFloat("Camera Height Ratio", &cameraHeightRatio, 0.0f, 1.0f))
				cameraHeightRatioChanged(cameraHeightRatio);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			
			ImGui::SliderFloat("Elevation", &elevation, 0.0f, 90.0f, "%3.0f (deg)", 1);		//slider which changed the elevation value by changing the slider
			ImGui::SliderFloat("Direction", &direction, 0.0f, 360.0f, "%3.0f (deg)", 1);	//slider which changes the direction with slider

			if (ImGui::Button("Aim")) {
				aim();
			}
			ImGui::SameLine();

			if (ImGui::Button("fire")) {	//cannon will fire when button hit
				fire();
			}
		}

		if (ImGui::Button("Reset")) reset();
		ImGui::SameLine();
		if (ImGui::Button(isRunning ? "Stop" : " Go ")) isRunning = !isRunning;
		ImGui::SameLine();
		ImGui::Text("   Time = %8.1f", t);
		if (ImGui::Button("Quit")) quit();


		if (ImGui::CollapsingHeader("Numerical Output")) {
		}

		if (ImGui::CollapsingHeader("Graphical Output")) {
		}

		
	}
	// store window size so that camera can ignore mouse clicks
	mainWindowRectangle.setPosition(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
	mainWindowRectangle.setSize(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
	ImGui::End();

}



void ofApp::drawLoggingWindow() {
	ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiSetCond_FirstUseEver);
	if (ImGui::Begin("Logging")) {
	}
	// store window size so that camera can ignore mouse clicks
	loggingWindowRectangle.setPosition(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
	loggingWindowRectangle.setSize(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
	ImGui::End();
}

//--------------------------------------------------------------
// GUI events and listeners
//--------------------------------------------------------------

void ofApp::keyPressed(int key) {

	float d = easyCam.getDistance();

	switch (key) {
	case 'd':
		std::cout << direction << endl;
		std::cout << elevation << endl;
		break;

		//        case 'h':                               // toggle GUI/HUD
		//           isGuiVisible = !isGuiVisible;
		//            break;
		//        case 'b':                               // toggle debug
		//            isDebugVisible = !isDebugVisible;
		//            break;
		//        case 'a':                               // toggle axis unit vectors
		//            isAxisVisible = !isAxisVisible;
		//            break;
		//        case '1':                               // toggle grids (X)
		//           isXGridVisible = !isXGridVisible;
		//            break;
		//        case '2':                               // toggle grids (Y)
		//            isYGridVisible = !isYGridVisible;
		//            break;
		//        case '3':                               // toggle grids (Z)
		//            isZGridVisible = !isZGridVisible;
		//            break;
		//        case 'g':                               // toggle ground
		//            isGroundVisible = !isGroundVisible;
		//            break;
		//        case 'u':                               // set the up vector to be up (ground to be level)
		//            easyCam.setTarget(ofVec3f::zero());
		//            break;
		//
		//        case 'S' :                              // save gui parameters to file
		//            gui.saveToFile("settings.xml");
		//
		//            break;
		//        case 'L' :                              // load gui parameters
		//            gui.loadFromFile("settings.xml");
		//            break;
		//
	case 'z':
		easyCam.setPosition(ofVec3f(0, cameraHeightRatio*d, d*sqrt(1.0f - cameraHeightRatio*cameraHeightRatio)) + easyCamTarget);
		easyCam.setTarget(easyCamTarget);
		break;
	case 'Z':
		easyCam.setPosition(0, 0, d);
		easyCam.setTarget(ofVec3f::zero());
		break;
	case 'x':
		easyCam.setPosition(d*sqrt(1.0f - cameraHeightRatio*cameraHeightRatio), cameraHeightRatio*d, 0);
		easyCam.setTarget(ofVec3f::zero());
		break;
	case 'X':
		easyCam.setPosition(ofVec3f(d, 0, 0) + easyCamTarget);
		easyCam.setTarget(easyCamTarget);
		break;
	case 'Y':
		easyCam.setPosition(ofVec3f(0.001, d, 0) + easyCamTarget);
		easyCam.setTarget(easyCamTarget);
		break;

	case 'f':                               // toggle fullscreen
											// BUG: window size is not preserved
		isFullScreen = !isFullScreen;
		if (isFullScreen) {
			ofSetFullscreen(false);
		}
		else {
			ofSetFullscreen(true);
		}
		break;


	//if the m key is pressed the cannon will fire for target
	case 'm':
		fire();
		break;
		// simulation specific stuff goes here

	

	}
}


void ofApp::cameraHeightRatioChanged(float & cameraHeightRatio) {

	float d = easyCam.getDistance();
	easyCam.setPosition(0, cameraHeightRatio*d, d*sqrt(1.0f - cameraHeightRatio*cameraHeightRatio));
	easyCam.setTarget(easyCamTarget);
}


void ofApp::quit() {
	ofExit();
}

//--------------------------------------------------------------
// Unused
//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {
	// easy camera should ignore GUI mouse clicks
	if (mainWindowRectangle.inside(x, y) || loggingWindowRectangle.inside(x, y))
		easyCam.disableMouseInput();
	else
		easyCam.enableMouseInput();
}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
