#include "ofApp.h"
using namespace glm;

//Adds a rectangle lying on the xy plane centered around the given position
static void addRect(ofFloatColor col, ofMesh& m, glm::vec3 pos, float w = 2.f, float h = 2.f) {
	float w_2 = w * 0.5;
	float h_2 = h * 0.5;
	int Nv = m.getVertices().size();
	m.setMode(OF_PRIMITIVE_TRIANGLES);
	m.addVertex(pos + vec3(-w_2, -h_2, 0.));
	m.addVertex(pos + vec3(w_2, -h_2, 0.));
	m.addVertex(pos + vec3(w_2, h_2, 0.));
	m.addVertex(pos + vec3(-w_2, h_2, 0.));
	m.addTriangle(Nv + 0, Nv + 1, Nv + 2);
	m.addTriangle(Nv + 0, Nv + 2, Nv + 3);

	//Adds colour to vertices
	for (auto& p : m.getVertices()) m.addColor(col);
}

//Adds a custom quad mesh with texture co-ordinates and normals
static void addQuad(ofMesh& m, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, ofFloatColor col)
{
	//Creates vertex's of quad
	m.addVertex(a);
	m.addVertex(b);
	m.addVertex(c);
	m.addVertex(d);

	//Adds texture co-ords
	m.addTexCoord(vec2(0, 0));
	m.addTexCoord(vec2(1, 0));
	m.addTexCoord(vec2(1, 1));
	m.addTexCoord(vec2(0, 1));

	//Triangles
	m.addTriangle(0, 1, 2);
	m.addTriangle(0, 2, 3);

	//Colour + Normals
	for (auto& p : m.getVertices()) m.addNormal(normalize(-p));
	for (auto& p : m.getVertices()) m.addColor(col);
}


//LIGHTING---------------------------------------------------------------------------------------------------------------------------
static std::string glslLighting() {
	return R"(
	struct Light 
			{
				vec3 pos;
				float strength;
				float halfDist;
				float ambient;
				vec3 diffuse;
				vec3 specular;
			};

	struct Material 
			{
				vec3 diffuse;
				vec3 specular;
				float shine;
			};

	struct LightFall 
			{
				vec3 diffuse;
				vec3 specular;
			};

	// In - place addition : a += b
	void addTo ( inout LightFall a , in LightFall b)
			{
				a. diffuse += b. diffuse;
				a. specular += b. specular;
			}

	// Compute light components falling on surface
	LightFall computeLightFall ( vec3 pos , vec3 N , vec3 eye , in Light lt , in Material mt )
		{
			vec3 lightDist = lt . pos - pos;
			float hh = lt . halfDist * lt . halfDist ;
			float atten = lt . strength * hh /( hh + dot ( lightDist , lightDist ));
			vec3 L = normalize (lightDist);
			// diffuse
			float d = max ( dot (N , L) , 0.) ;
			d += lt . ambient ;
			// specular
			vec3 V = normalize ( eye - pos );
			vec3 H = normalize (L + V);
			float s = pow ( max ( dot (N , H) , 0.) , mt . shine );
			LightFall fall ;
			fall . diffuse = lt . diffuse *( d* atten );
			fall . specular = lt . specular *( s* atten );
			return fall ;
		}

	// Get final color reflected off material
	vec3 lightColor (in LightFall f , in Material mt )
		{
			return f. diffuse * mt . diffuse + f. specular * mt . specular ;
		}

	)";
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
void ofApp::setup() {

	//Camera setup
	  cam.setPosition(vec3(0., 0., 2.7));
	  cam.setNearClip(0.05);
	  cam.setFarClip(100.);

	//BG
	  backgroundMesh = ofMesh::sphere(cam.getFarClip() * 0.85);
	  if (!background.load("background.jpg")) std::cout << " Error loading envir map " << std::endl;

	//Sound
	  song.load("Happy_Birthday.wav");

	//3D setup
	  ofEnableDepthTest();
	  ofSetFrameRate(40);
	  ofDisableArbTex();
	  ofEnableNormalizedTexCoords();

	//Image loading
	  if (!image.load("icing.jpg")) std::cout << " Error loading image file " << std::endl;
	  if (!image2.load("polkaDot.jpg")) std::cout << " Error loading image file " << std::endl;
	  if (!image3.load("spongeCake.jpg")) std::cout << " Error loading image file " << std::endl;
	  if (!image4.load("paperTexture.jpg")) std::cout << " Error loading image file " << std::endl;
	  if (!image5.load("icing2.jpg")) std::cout << " Error loading image file " << std::endl;
	  if (!image6.load("chocolateSponge.jpeg")) std::cout << " Error loading image file " << std::endl;
	
	
	//Model loading and initialising
	  mainCake.loadModel("mainCake.dae");
	  mainCake.disableMaterials();

	  cakeSponge.loadModel("cakeSponge.dae");
	  cakeSponge.disableMaterials();

	  cream1.loadModel("cream1.dae");
	  cream1.disableMaterials();

	  cream2.loadModel("cream2.dae");
	  cream2.disableMaterials();

	  cakeSlice.loadModel("cakeSlice.dae");
	  cakeSlice.disableMaterials();

	  sliceSponge.loadModel("sliceSponge.dae");
	  sliceSponge.disableMaterials();

	  cakeKnife.loadModel("cakeKnife.dae");
	  cakeKnife.disableMaterials();

	  candle.loadModel("candle.dae");
	  candle.disableMaterials();

	  plate.loadModel("plate.dae");
	  plate.disableMaterials();

	//Variable setup
	  candlesOn = true;
	  vanillaCake = true;
	  animationY.para();

	//CUSTOM TEXTURE SHADER--------------------------------------------------------------------------------------------------
	build(textureShader, R"(
		// Vertex program
		uniform mat4 modelViewProjectionMatrix;
		uniform mat4 projectionMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 modelMatrix;

		in vec2 texcoord; // texture coordinate ( vertex attribute from mesh )
		in vec4 position;
		in vec3 normal;
		in vec3 color;

		out vec3 vposition;
		out vec3 vnormal;
		out vec3 vcolor;
		out vec2 vtexcoord; // passed to fragment shader

		void main () 
			{
				vtexcoord = texcoord ;
				vcolor = color ;
				vnormal = normal ;
				vposition = ( modelMatrix * position ). xyz ;
				gl_Position = projectionMatrix * viewMatrix * vec4 ( vposition , 1.) ;
			}
		)", glslLighting() + R"(
		//Fragment program
		uniform vec3 eye;
		uniform sampler2D tex; // texture (ID) passed in from the CPU
		uniform float texturing;

		in vec3 vposition;
		in vec3 vnormal;
		in vec3 vcolor;
		in vec2 vtexcoord; // interpolant from vertex shader

		out vec4 fragColor;
		
		void main() 
			{
				vec3 pos = vposition ;
				vec3 normal = normalize ( vnormal );

				//First light, white, positioned in top right corner.
				Light light1 ;
				light1.pos = vec3 (0.5 , 1.5 , -0.5) ;
				light1 . strength = 1.5;
				light1 . halfDist = 1.;
				light1 . ambient = 0.8;
				light1 . diffuse = vec3 (1. ,1. ,1.) ;
				light1 . specular = light1 . diffuse ;

				//Second light, blue, positioned in bottom right corner.
				Light light2 = light1 ;
				light2 . pos = vec3 (0. , -0.95 ,0.) ;
				light2 . diffuse = vec3 (0. ,0. ,1.) ;
				light2 . specular = light2 . diffuse ;
				
				//Third light, white, positioned in bottom left corner.
				Light light3 = light1 ;
				light3.strength = 0.7;
				light3 . pos = vec3 (-0.7 , -0.6 ,0.) ;
				light3 . diffuse = vec3 (1. ,1. ,1.) ;
				light3 . specular = light1 . diffuse ;

				Material mtrl ;
				mtrl . diffuse = texture ( tex , vtexcoord ).rgb ;
				mtrl . specular = vec3 (1.) ;
				mtrl . shine = 100.;
				LightFall fall = computeLightFall ( pos , normal , eye , light1 , mtrl );
				addTo ( fall , computeLightFall ( pos , normal , eye , light2 , mtrl ));
				addTo ( fall , computeLightFall ( pos , normal , eye , light3 , mtrl ));
				vec3 col = lightColor ( fall , mtrl );
		
				col = mix ( vcolor , col , texturing );
				fragColor = vec4 ( col , 1.);
			}
	)");
	//Texture images
	  auto& tex = image.getTexture();
	  auto& tex2 = image2.getTexture();
	  auto& tex3 = image3.getTexture();
	  auto& tex4 = image4.getTexture();
	  auto& tex5 = image5.getTexture();
	  auto& tex6 = image6.getTexture();

	//CUSTOM REFLECTION SHADER-----------------------------------------------------------------------------
	build(mirrorShader, R"(
		//Vertex program
		uniform mat4 projectionMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 modelMatrix;

		in vec4 position;
		in vec3 normal;
		in vec3 color;

		out vec3 vposition;
		out vec3 vnormal;
		out vec3 vcolor;

		void main () 
			{
				vcolor = color;
				vnormal = normal;
				vposition = ( modelMatrix * position ).xyz;
				gl_Position = projectionMatrix * viewMatrix * vec4 ( vposition , 1.);
			}

		)", glslLighting() + R"(
		//Fragment program
		uniform vec3 eye ;
		uniform sampler2D background;

		in vec3 vposition;
		in vec3 vnormal;
		in vec3 vcolor;

		out vec4 fragColor;

		vec3 calcReflection ( vec3 I , vec3 N , in sampler2D background )
			{
                    vec3 rayDir = reflect (I , N);
                    // Convert ray direction from Cartesian to polar coords
                    float theta = atan ( rayDir.x , rayDir.z);
                    float phi = asin ( rayDir.y);
                    // Map polar coord to texture coord in the environment map
                    const float oneOverPi = 113./355.;
                    vec2 uv = vec2(0.5 + 0.5* theta * oneOverPi , 0.5 - phi * oneOverPi );
                    // Lookup the color from the environment map
                    return texture (background , uv).rgb;
              }

		void main () 
			{
				vec3 pos = vposition;
				vec3 normal = normalize (vnormal);
				
				//First light, white, positioned in top right corner.
				Light light1 ;
				light1.pos = vec3 (0.5 , 1.5 , -0.5) ;
				light1 . strength = 1.5;
				light1 . halfDist = 1.;
				light1 . ambient = 0.8;
				light1 . diffuse = vec3 (1. ,1. ,1.) ;
				light1 . specular = light1 . diffuse ;

				//Second light, blue, positioned in bottom right corner.
				Light light2 = light1 ;
				light2 . pos = vec3 (0. , -0.95 ,0.) ;
				light2 . diffuse = vec3 (0. ,0. ,1.) ;
				light2 . specular = light2 . diffuse ;
				
				//Third light, white, positioned in bottom left corner.
				Light light3 = light1 ;
				light3.strength = 0.7;
				light3 . pos = vec3 (-0.7 , -0.6 ,0.) ;
				light3 . diffuse = vec3 (1. ,1. ,1.) ;
				light3 . specular = light1 . diffuse ;

				Material mtrl;
				mtrl.diffuse = vec3(0.2, 0.4, 0.7).rgb;
				mtrl.specular = vec3(1.,1.,1.);
				mtrl.shine = 200.;
				LightFall fall = computeLightFall ( pos , normal , eye , light1 , mtrl );
				addTo ( fall , computeLightFall ( pos , normal , eye , light2 , mtrl ));
				addTo ( fall , computeLightFall ( pos , normal , eye , light3 , mtrl ));
				vec3 col = lightColor ( fall , mtrl );
				vec3 I = - normalize ( eye - pos ); // incident ray

				//Adds reflection
				float reflectivity = 0.7;
				vec3 reflCol = calcReflection (I , normal , background );
				col = mix ( col , reflCol , reflectivity );
				fragColor = vec4 ( col , 1.);
			}	
	)");


	//POINT SPRITES SHADER-----------------------------------------------------------------------------------
	build(pointShader, R"(
			//Vertex program
			uniform mat4 modelViewProjectionMatrix;
			uniform float spriteRadius;
			uniform mat4 cameraMatrix;
		
			in vec3 color;
			in vec4 position;

			out vec3 vcolor;
			out vec2 vtexcoord ; // passed to fragment shader
			out vec2 spriteCoord; // sprite coordinate in [ -1 ,1]

			void main() 
				{
					vec4 pos = position;
					switch ( gl_VertexID % 4) 
					{
						case 0: spriteCoord = vec2 ( -1. , -1.) ; break;
						case 1: spriteCoord = vec2 ( 1. , -1.) ; break;
						case 2: spriteCoord = vec2 ( 1. , 1.) ; break;
						case 3: spriteCoord = vec2 ( -1. , 1.) ; break;
					}

					vcolor = color;
					vec4 offset = vec4 ( spriteCoord * spriteRadius , 0. , 0.);
					offset = cameraMatrix * offset;
					pos += offset;
					vtexcoord = spriteCoord;
					gl_Position = modelViewProjectionMatrix * pos;
				}

			)", R"(
			// Fragment program
			uniform sampler2D tex; 
			uniform float texturing ;

			in vec3 vcolor ;
			in vec2 vtexcoord ;
			in vec2 spriteCoord; //sprite coordinate in [ -1 ,1]

			out vec4 fragColor;
			void main() 
				{
					float rsqr = dot( spriteCoord, spriteCoord);
					if( rsqr > 1.) discard;
					//vec3 col = vec3(1,1,0);
					//vec3 col = texture(tex, vtexcoord).rgb;
					vec3 col = texture ( tex , vtexcoord ). rgb ;
					col = mix ( vcolor , col , texturing );

					float a = 1. - rsqr ; // inverted parabola
					float w = 0.5; // attenuation width
					float wsqr = w*w;
					a *= wsqr /( wsqr + rsqr );
					col *= a;
					fragColor = vec4(col, 1.);
				}
		)");

	//Noise texture
	  int W = 512, H = W;
	  auto format = GL_LUMINANCE;
	  ofPixels pix; // 2D array of unsigned char
	  pix.allocate(W, H, OF_PIXELS_GRAY);
	  for (int j = 0; j < H; ++j)
	  {
		for (int i = 0; i < W; ++i)
		{
			auto uv = vec2(i, j) / (vec2(W, H) - 1.f);
			float val = 0.;
			int octaves = 6;
			float f = 2.; // starting frequency
			float Asum = 0.; // used to scale final amplitude into [0,1]
			for (int k = 1; k <= octaves; ++k)
			{
				float A = 1. / f;
				val += A * ofNoise(uv * f);
				f *= 2.f;
				Asum += A;
			}
			pix[j * W + i] = val / Asum * 255;
		}
	  }
	  noiseTex.allocate(pix);

	//Point sprite meshes
	  for (int i = 0; i < 10; ++i)
	  {
		  auto p = vec3(ofRandom(0.05), ofRandom(0.05), ofRandom(0.05));
		  addRect(ofFloatColor(1, 0, 0),pointMesh, p, 0, 0);
	  }

	  for (int i = 0; i < 100; ++i)
	  {
		  auto p2 = vec3(ofRandom(3.), ofRandom(3.), ofRandom(3.));
		  addRect(ofFloatColor(0, 0, 1),pointMesh2, p2, 0, 0);
	  }

	//Box walls
	  addQuad(wall1, vec3(1.5, -0.6, -1.5), vec3(-1.5, -0.6, -1.5), vec3(-1.5, 1.5, -1.5), vec3(1.5, 1.5, -1.5), ofFloatColor(1, 1, 1));
	  addQuad(wall2, vec3(-1.5, -0.6, -1.5), vec3(-1.5, -0.6, 1.6), vec3(-1.5, 1.5, 1.6), vec3(-1.5, 1.5, -1.5), ofFloatColor(1, 1, 1));
	  addQuad(wall3, vec3(1.5, -0.6, -1.5), vec3(1.5, -0.6, 1.6), vec3(1.5, 1.5, 1.6), vec3(1.5, 1.5, -1.5), ofFloatColor(1, 1, 1));
	  addQuad(floor, vec3(1.5, -0.6, -1.5), vec3(-1.5, -0.6, -1.5), vec3(-1.5, -0.6, 1.6), vec3(1.5, -0.6, 1.6), ofFloatColor(1, 1, 1));
	
}

//--------------------------------------------------------------
void ofApp::update() {
	//Delta seconds of last frame render
	float dt = ofGetLastFrameTime()/ 10;

	//Update animations
	input += 0.1;
	for (auto& a : { &animationY })
		a->update(dt);
}

//--------------------------------------------------------------
void ofApp::draw() {
	
	//Animation updates
	  float cakeSliceY = animationY.para();
	  mappedSin = ofMap(sin(input), -1, 1, -0.012, 0.012);

	//Setup
	  cam.begin();
	  ofEnableLighting();

	//Texture shader
	  textureShader.begin();
	  if (vanillaCake)
	  {
		  textureShader.setUniformTexture("tex", image, 0);
	  }
	  else
	  {
  		  textureShader.setUniformTexture("tex", image5, 0);
	  }
	  textureShader.setUniform1f("texturing", 1.);
	  textureShader.setUniform3f("eye", cam.getPosition());

	//Main cake icing
	  ofPushMatrix();
	  ofTranslate(0, -0.5, 0);
	  ofRotate(200, 0, 1, 0);
	  ofScale(0.005);
	  mainCake.drawFaces();
	  ofPopMatrix();

	
	  for (int i = 0; i < 28; i++)
	  {
		 ofPushMatrix();
		  ofTranslate(0., -0.5, 0);
		  ofRotate(57 + 360 / 32.9 * i, 0, 1, 0);
		  ofScale(0.00058);
		  cream2.drawFaces();
		  ofPopMatrix();
	  }

	  for (int i = 0; i < 6; i++)
	  {
		  ofPushMatrix();
		  ofTranslate(0, -0.37, 0);
		  ofRotate(70 + 360 / 7 * i, 0, 1, 0);
		  ofScale(0.0009);
		  cream1.drawFaces();
		  ofPopMatrix();
	  }

	//Cake slice icing
	  ofPushMatrix();
	  ofTranslate(-0.1, -0.45 + cakeSliceY, -0.2);
	  ofRotate(200, 0, 1, 0);
	  ofScale(0.0025);
	  cakeSlice.drawFaces();
	  ofPopMatrix();

	  ofPushMatrix();
	  ofTranslate(0, -0.37 + cakeSliceY, 0.40);
	  ofRotate(37, 0, 1, 0);
	  ofScale(0.0009);
	  cream1.drawFaces();
	  ofPopMatrix();


	  for (int i = 0; i < 4; i++)
	  {
		  ofPushMatrix();
		  ofTranslate(0.1, -0.47 + cakeSliceY, 0.25);
		  ofRotate((i * 12), 0, 1, 0);
		  ofRotate(3.8, 0, 1, 0);
		  ofScale(0.00058);
		  cream2.drawFaces();
		  ofPopMatrix();
	  }
	
	  if (vanillaCake)
	  {
		  textureShader.setUniformTexture("tex3", image3, 0);
	  }
	  else
	  {
		  textureShader.setUniformTexture("tex6", image6, 0);
	  }

	//Main cake sponge
	  for (int i = 0; i < 2; i++)
	  {
		  ofPushMatrix();
		  ofTranslate(0, -0.5 + (i*-0.475), 0);
		  ofRotate(200, 0, 1, 0);
		  ofScale(0.0025);
		  cakeSponge.drawFaces();
		  ofPopMatrix();
	  }

	  for (int i = 0; i < 2; i++)
	  {
		  ofPushMatrix();
		  ofTranslate(0, -0.5 + (i * -0.475), 0);
		  ofRotate(211, 0, 1, 0);
		  ofScale(-0.0025,0.0025,0.0025);
		  cakeSponge.drawFaces();
		  ofPopMatrix();
	  }
	
	//Cake slice sponge
	  for (int i = 0; i < 2; i++)
	  {
		  ofPushMatrix();
		  ofTranslate(-0.1, -0.45 + (i * -0.475) + cakeSliceY, -0.2);
		  ofRotate(200, 0, 1, 0);
		  ofScale(0.0025);
		  sliceSponge.drawFaces();
		  ofPopMatrix();
	  }

    //Plate
	  textureShader.setUniformTexture("tex2", image2, 0);
	  ofPushMatrix();
	  ofTranslate(0, -0.55, 0);
	  ofRotate(200, 0, 1, 0);
	  ofScale(-0.007);
	  plate.drawFaces();
	  ofPopMatrix();

	//Candles
	  for (int i = 0; i < 6; i++)
	  {
		  ofPushMatrix();
		  ofTranslate(0, -0.37, 0);
		  ofRotate(70 + 360 / 7 * i, 0, 1, 0);
		  ofPushMatrix();
		  ofTranslate(0, 0.2, 0.08);
		  ofScale(0.0009);
		  candle.drawFaces();
		  ofPopMatrix();
		  ofPopMatrix();
	  }
	
	//Box walls
	  textureShader.setUniformTexture("tex4", image4, 0);
	  wall1.draw();
	  wall2.draw();
	  wall3.draw();
	  floor.draw();
	  textureShader.end();

	//Mirror shader
	  mirrorShader.begin();
	  mirrorShader.setUniform3f("eye", cam.getPosition());
	  mirrorShader.setUniformTexture("background", background, 0);

    //Cake knife
	  ofPushMatrix();
	  ofTranslate(-0.05, -0.47 + cakeSliceY, -0.05);
	  ofRotate(200, 0, 1, 0);
	  ofScale(0.005);
	  cakeKnife.drawFaces();
	  ofPopMatrix();
	  mirrorShader.end();
	
	//Disabling
	  ofDisableLighting();
	  glDepthMask(GL_FALSE);
	  ofEnableBlendMode(OF_BLENDMODE_ADD);

	//Point sprite shader
	  pointShader.begin();
	  pointShader.setUniformTexture("tex", noiseTex, 0);
	  pointShader.setUniformMatrix4f("cameraMatrix", cam.getLocalTransformMatrix());
	  pointShader.setUniform1f("spriteRadius", 0.1);
	  pointShader.setUniform1f("texturing", 0.4);
	
	  //Candle 'flames'
	  if (candlesOn)
	  {
		  for (int i = 0; i < 6; i++)
		  {
			  ofPushMatrix();
			  ofRotate(360 / 7 * i, 0, 1, 0);
			  ofPushMatrix();
			  ofTranslate(0.6 + mappedSin, 0.80, 0.2);
			  pointMesh.draw();
			  ofPopMatrix();
			  ofPopMatrix();
		  }
	  }
	  //Sparkles
	  ofTranslate(-1.5, -1.5, -1.5);
	  pointMesh2.draw();
	  pointShader.end();
	  ofDisableBlendMode();
	  glDepthMask(GL_TRUE);

	//Background
	  background.bind();
	  backgroundMesh.draw();
	  background.unbind();
	  cam.end();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	//'Blows' out candles when space is pressed. Relights if pressed again.
	if (key == 32)
	{
		candlesOn =! candlesOn;
	}

	//Changes cake flavour when 'c' key is pressed.
	if (key == 99)
	{
		vanillaCake = !vanillaCake;
	}
	
	//Plays happy birthday song when 'p' is pressed
	if (key == 112)
	{
		song.play();
	}


}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

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

