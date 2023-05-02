#pragma once

#include "ofMain.h"
#include "ofGraphicsUtil.h"
#include "ofxAssimpModelLoader.h"
#include "PRamp.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
		//Camera
		ofEasyCam cam;

		//Images
		ofImage image;
		ofImage image2;
		ofImage image3;
		ofImage image4;
		ofImage image5;
		ofImage image6;
		ofImage background;

		//Shaders
		ofShader textureShader;
		ofShader mirrorShader;
		ofShader pointShader;

		//Meshes
		ofMesh wall1;
		ofMesh wall2;
		ofMesh wall3;
		ofMesh floor;
		ofMesh pointMesh;
		ofMesh pointMesh2;
	
		//3D model meshes
		ofxAssimpModelLoader mainCake;
		ofxAssimpModelLoader cakeSponge;
		ofxAssimpModelLoader cream1;
		ofxAssimpModelLoader cream2;
		ofxAssimpModelLoader cakeSlice;
		ofxAssimpModelLoader sliceSponge;
		ofxAssimpModelLoader cakeKnife;
		ofxAssimpModelLoader candle;
		ofxAssimpModelLoader plate;

		//Textures
		ofTexture noiseTex;

		//Variables
		float input;
		float mappedSin;
		bool candlesOn;
		bool vanillaCake;

		//Misc
		PRamp animationY;
		ofVboMesh backgroundMesh;
		ofSoundPlayer song;



};
