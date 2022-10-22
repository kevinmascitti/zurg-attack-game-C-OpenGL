// assimp include files. These three are usually needed.
#include "assimp.h"
#include "aiPostProcess.h"
#include "aiScene.h"

#include "iostream.h"
#include "string.h"
#include <chrono>
#include <map>
#include "Windows.h"
using namespace std;

#include <stdio.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#include "GL/glut.h"
#include <IL/il.h>

#include "Sound.h"

// operazioni matematica e vettori 
#include<vector>
#include <cmath>

//to map image filenames to textureIds
#include <string.h>
#include <map>


/* --------------------------------------------------------------

	 ZURG ATTACK - un videogame arcade ispirato a Toy Story.

	 Progetto realizzato per il corso di INFORMATICA GRAFICA
	 con l'ausilio delle librerie OpenGL, Assimp e DevIL.

	 Politecnico di Torino - anno accademico 2021/22
	 Docenti:
	 Fabrizio Lamberti
	 Alberto Cannavò

	 Ideato e realizzato dagli studenti:
	 Federico Mafrici, s306156
	 Kevin Mascitti, s302286

--------------------------------------------------------------- */







// currently this is hardcoded
//static const std::string basepath = "./models/textures/"; //obj..
static const std::string basepath = "./models/"; //per i file blend

// the global Assimp scene object
const struct aiScene* scene = NULL;
struct aiVector3D scene_min, scene_max, scene_center,bounding_boxes;

// current rotation angle
static float angle = 0.f;

// images / texture
std::map<std::string, GLuint*> textureIdMap;	// map image filenames to textureIds
GLuint* textureIds;							// pointer to texture Array

GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 2.0f, 0.0f, 15.0f, 1.0f };

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)
#define initWIDTH	1080
#define initHEIGHT	600
#define initHP 150
#define initPUNT 0
#define STDDiff 0.65	// facile 0.75 normale 0.65 facile 0.55
#define STDZurg 1
#define MAX 10
#define AstronaveZ 950

//---------- VARIABILI USATE
// GESTIONE GIOCO 
int initi = 0;
int objectCount = 0;
int contatoreMonete = 0;
int contatoreCubo = 0;
int statoGioco = 3;
int Width = initWIDTH;
int Height = initHEIGHT;
int boss = 0;
int posizioni[13] ={-2100,-1800,-1500,-1200,-900,-600,-300,0,300,600,900,1200,1500};
bool mButtonPressed = false;
float mouseX, mouseY;

//--------- VARIABILI PER IL MOVIMENTO 
int InitialZ = 2000;
int InitialY = 1100;
int prev_time3 = 0;
int numeroPartite = 0;

class ObjectScene
{
	public:
	int ID = -1;
	float X=0;
	float height=0;
	float Z=0;
	struct aiVector3D Vmin;
	struct aiVector3D Vmax;
	int collisione = 0;

	ObjectScene()
	{
		ID = -1;
		X = 0;
		height = 0;
		Z = 0;
		collisione = 0;
		Vmin.x = Vmin.y = Vmin.z = 1e10f;
		Vmax.x = Vmax.y = Vmax.z = -1e10f;
	}

};
//--------- VARIAIBILI DI STATO DI GIOCO
std::string vita = "";
std::string powerUp = "";
std::string punteggio = std::to_string(initPUNT);
std::string pausa = "p: pausa";
int HP = initHP;
int Punteggio = initPUNT;
float moltiplicatore = 1.0;
int invincibile = 0;
int powerUpId = -1;
int livello = 2;
float difficolta = STDDiff;
int difficoltaZurg = STDZurg;
int firstDisplay = 0;
int displayBoss = 0;
int change = 0;
int powerUpAttivo = 0;
int musicON = 1;
Sound effects;
int effectON = 1;
int abilitaGenerazioneOggetto = 1;
int abilitaGenerazioneBoss = 0;
int visuale = 1;
int glitch = 0;
int primaChiamataOggetto = 1;
int primaChiamataBoss = 1;
std::map<int,std::string, greater<int>> record;

//VETTORE DI OGGETTI :
ObjectScene oggetti[3000];
ObjectScene astronave;
ObjectScene zurg;
ObjectScene laser;

// GESTIONE VARIABILI DI STATO VITA E POWER UP
int generaPowerUp() 
{	
	effects.Play(CUBE);
	powerUpId = rand() % 3;
	switch (powerUpId)
	{
	case 0: // invincibilità 
		powerUp = "Invincibilita'";
		break;
	case 1: // 2X doppi punti
		powerUp = "2X";
		break;
	case 2: // bomba
		powerUp = "Bomba";
		break;
	default:
		break;
	}
	return powerUpId;
}

void stopPowerUp(int powerUpidentificatore)
{
	switch (powerUpidentificatore)
	{
	case 0: // invincibilità 
		powerUp = "";
		powerUpId = -1;
		invincibile = 0;
		powerUpAttivo = 0;
		break;
	case 1: // 2X doppi punti
		powerUp = "";
		powerUpId = -1;
		moltiplicatore = 1.0;
		powerUpAttivo = 0;
		break;
	case 2: // bomba
		powerUp = "";
		powerUpId = -1;
		powerUpAttivo = 0;
		break;
	default:
		powerUpId = -1;
		break;
	}
}

int attivaPowerUp(int powerUpId) 
{
	if(effectON)
	effects.Play(POWERUP);
	switch (powerUpId)
	{
	case 0: // invincibilità 
		invincibile = 1;
		break;
	case 1: // 2X doppi punti
		moltiplicatore = 2.0;
		break; 
	case 2: // bomba elimina tutti gli oggetti e il laser
		for (int i = 0; i < objectCount; i++)
				oggetti[i].ID = -1;
		laser.ID = -1;
		objectCount = 0;
		break;
	default:
		break;
	}
	return powerUpId;
}
// 5 astronave ecco perchè non messo 
// 7 sfondo 
// 1-6 collissione con oggetti 
// 7 punteggio 
// 8-9 power up/  Vita 
void gameManager(int ID)
{
	switch (ID)
	{
	case 1:
		if (effectON)
		effects.Play(HIT);
		if (!invincibile)
			HP -= 25;
		if (HP <= 0) {
			if (effectON)
			effects.Play(GAMEOVER);
			statoGioco = 4;
			glutPostRedisplay();
		}
		break;
	case 2:
		if (effectON)
		effects.Play(HIT);
		if (!invincibile)
			HP -= 25;
		if (HP <= 0) {
			if (effectON)
			effects.Play(GAMEOVER);
			statoGioco = 4;
			glutPostRedisplay();
		}
		break;
	case 3:
		if (effectON)
		effects.Play(HIT);
		if (!invincibile)
			HP -= 25;
		if (HP <= 0) {
			if (effectON)
			effects.Play(GAMEOVER);
			statoGioco = 4;
			glutPostRedisplay();
		}
		break;
	case 4:
		if (effectON)
		effects.Play(HIT);
		if (!invincibile)
			HP -= 30;
		if (HP <= 0) {
			if (effectON)
			effects.Play(GAMEOVER);
			statoGioco = 4;
			glutPostRedisplay();
		}
		break;
	case 6:
		if (effectON)
		effects.Play(HIT);
		if (!invincibile)
			HP -= 25;
		if (HP <= 0) {
			if (effectON)
			effects.Play(GAMEOVER);
			statoGioco = 4;
			glutPostRedisplay();
		}
		break;
	case 8:
		if (effectON)
		effects.Play(COIN);
		Punteggio += 50 * moltiplicatore;
		glutPostRedisplay();
		break;
	case 9:
		if (powerUpId == -1)
			powerUpId = generaPowerUp();
		break;
	case 10:
		if (effectON)
		effects.Play(HEART);
		if(HP < 150)
		HP += 25;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

void passiveMotionFunc(int x, int y)
{
	//when mouse not clicked
	mouseX = 2*(float(x) / (Width / 1200.0) -600.0);  //converting screen resolution to ortho 2d spec
	mouseY = -2*(float(y) / (Height / 700.0) - 350.0);
	glutPostRedisplay();
}

void mouseClick(int buttonPressed, int state, int x, int y)
{
	if (buttonPressed == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mButtonPressed = true;
	}
	else {
		mButtonPressed = false;
		glitch = 0;
	}
	glutPostRedisplay();
}

void checkDifficolta()
{
	if (Punteggio > 300 && change == 0)
	{
		difficolta -= 0.15;
		change++;
		return;
	}
	if (Punteggio > 600 && change == 1)
	{
		difficoltaZurg -= 0.15;
		change++;
		return;
	}
	if (Punteggio > 900 && change == 2)
	{
		difficolta -= 0.15;
		change++;
		return;
	}
}

void refresh() {
	checkDifficolta();
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	float  spostamento = 300.0;
	if (statoGioco == 2)
	{
		switch (key)
		{
		case 'a':
			if (astronave.X - spostamento > -2300)
			{
				astronave.X = astronave.X - spostamento;
				astronave.Vmin.x -= spostamento;
				astronave.Vmax.x -= spostamento;
				glutPostRedisplay();
			}
			break;
		case 'A':
			if (astronave.X - spostamento > -2300)
			{
				astronave.X = astronave.X - spostamento;
				astronave.Vmin.x -= spostamento;
				astronave.Vmax.x -= spostamento;
				glutPostRedisplay();
			}
			break;
		case 'd':
			if (astronave.X + spostamento < 2300)
			{
				astronave.X = astronave.X + spostamento;
				astronave.Vmin.x += spostamento;
				astronave.Vmax.x += spostamento;
				glutPostRedisplay();

			}
			break;
		case 'D':
			if (astronave.X + spostamento < 2300)
			{
				astronave.X = astronave.X + spostamento;
				astronave.Vmin.x += spostamento;
				astronave.Vmax.x += spostamento;
				glutPostRedisplay();

			}
			break;
		case ' ':
			if (powerUpId != -1 && powerUpAttivo == 0)
			{
				attivaPowerUp(powerUpId);
				if (powerUpId != 2)
				{
					glutTimerFunc(8000, stopPowerUp, powerUpId);
					powerUpAttivo = 1;
				}
				else 
					stopPowerUp(2);
			}
			break;
		case 'p':
			if (effectON && !boss && !displayBoss)
			{
				effects.Play(CHOOSE);
				statoGioco = 7;
			}
			break;
		case 'P':
			if (effectON && !boss && !displayBoss)
			{
				effects.Play(CHOOSE);
				statoGioco = 7;
			}
			break;
		default:
			break;
		}

	}
	else if(statoGioco==3) {
		switch(key){
		case ' ':
			if (effectON)
			effects.Play(CHOOSE);
			statoGioco = 1;
			break;
		default:
			break;
		}
	}
	else if(statoGioco==4) {
		switch(key){
		case ' ':
			if (effectON)
			effects.Play(CHOOSE);
			numeroPartite++;
			initi = 0;
			if(musicON)
			PlaySound(TEXT("ZurgTheme.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
			statoGioco = 2;
			break;
		default:
			break;
		}
	}
	glutPostRedisplay();
}
/*
void render()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const double w = glutGet(GLUT_WINDOW_WIDTH);
	const double h = glutGet(GLUT_WINDOW_HEIGHT);
	gluPerspective(45.0, w / h, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -15);

	glutSwapBuffers();
}
*/
void displayRasterText(float x, float y, float z, char* stringToDisplay) {
	glRasterPos3f(x, y, z);
	for (char* c = stringToDisplay; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}
}

void displayRasterText2D(float x, float y, char* stringToDisplay) {
	glRasterPos2f(x, y);
	for (char* c = stringToDisplay; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}
}

bool ChangeVolume(double nVolume, bool bScalar)
{
	HRESULT hr = NULL;
	bool decibels = false;
	bool scalar = false;
	double newVolume = nVolume;

	CoInitialize(NULL);
	IMMDeviceEnumerator* deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);
	IMMDevice* defaultDevice = NULL;

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	deviceEnumerator->Release();
	deviceEnumerator = NULL;

	IAudioEndpointVolume* endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume),
		CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&endpointVolume);
	defaultDevice->Release();
	defaultDevice = NULL;

	// -------------------------
	float currentVolume = 0;
	endpointVolume->GetMasterVolumeLevel(&currentVolume);
	//printf("Current volume in dB is: %f\n", currentVolume);

	hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
	//CString strCur=L"";
	//strCur.Format(L"%f",currentVolume);
	//AfxMessageBox(strCur);

	// printf("Current volume as a scalar is: %f\n", currentVolume);
	if (bScalar == false)
	{
		hr = endpointVolume->SetMasterVolumeLevel((float)newVolume, NULL);
	}
	else if (bScalar == true)
	{
		hr = endpointVolume->SetMasterVolumeLevelScalar((float)newVolume, NULL);
	}
	endpointVolume->Release();

	CoUninitialize();

	return FALSE;
}

void reshape(int width, int height)
{
	Width = width;
	Height = height;
	const double aspectRatio = (float)width / height, fieldOfView = 45.0;
	if (statoGioco == 2) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fieldOfView, aspectRatio,
			1.0, 1000.0);  // Znear and Zfar 
		glViewport(0, 0, width, height);
		glPopMatrix();
	}
	else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1200, 1200, -700, 700);                   //<-----CHANGE THIS TO GET EXTRA SPACE
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}

void get_bounding_box_for_node(const struct aiNode* nd,
	struct aiVector3D* min,
	struct aiVector3D* max,
	struct aiMatrix4x4* trafo
) {
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo, &nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, trafo);

			min->x = aisgl_min(min->x, tmp.x);
			min->y = aisgl_min(min->y, tmp.y);
			min->z = aisgl_min(min->z, tmp.z);

			max->x = aisgl_max(max->x, tmp.x);
			max->y = aisgl_max(max->y, tmp.y);
			max->z = aisgl_max(max->z, tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n], min, max, trafo);
	}
	*trafo = prev;
}

void get_bounding_box(struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z = 1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode, min, max, &trafo);
}

void color4_to_float4(const struct aiColor4D* c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

void apply_material(const struct aiMaterial* mtl)
{
	float c[4];

	GLenum fill_mode;
	int ret1, ret2;
	struct aiColor4D diffuse;
	struct aiColor4D specular;
	struct aiColor4D ambient;
	struct aiColor4D emission;
	float shininess, strength;
	int two_sided;
	int wireframe;
	int max;

	int texIndex = 0;
	aiString texPath;	//contains filename of texture
	if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath))
	{
		//bind texture
		unsigned int texId = *textureIdMap[texPath.data];
		glBindTexture(GL_TEXTURE_2D, texId);
	}

	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		color4_to_float4(&diffuse, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
		color4_to_float4(&specular, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
		color4_to_float4(&ambient, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, (unsigned int*)&max);
	max = 1;
	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, (unsigned int*)&max);
	if ((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
	else {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	max = 1;
	if (AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, (unsigned int*)&max))
		fill_mode = wireframe ? GL_LINE : GL_FILL;
	else
		fill_mode = GL_FILL;
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	max = 1;
	if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, (unsigned int*)&max)) && two_sided)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

// ----------------------------------------------------------------------------

// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
void Color4f(const struct aiColor4D* color)
{
	glColor4f(color->r, color->g, color->b, color->a);
}

// ----------------------------------------------------------------------------

void recursive_render(const struct aiScene* sc, const struct aiNode* nd, float scale)
{
	unsigned int i;
	unsigned int n = 0, t;
	struct aiMatrix4x4 m = nd->mTransformation;

	//printf("Node name: %s\n", nd->mName.data);

	//m.Scaling(aiVector3D(scale, scale, scale), m);

	// update transform
	m.Transpose();
	glPushMatrix();
	glMultMatrixf((float*)&m);

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n)
	{
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

		///
		//printf("Drawing MESH with this name: %s %d \n", mesh->mName.data,objectCount);

		apply_material(sc->mMaterials[mesh->mMaterialIndex]);


		if (mesh->HasTextureCoords(0))
			glEnable(GL_TEXTURE_2D);
		else
			glDisable(GL_TEXTURE_2D);
		if (mesh->mNormals == NULL)
		{
			glDisable(GL_LIGHTING);
		}
		else
		{
			glEnable(GL_LIGHTING);
		}

		if (mesh->mColors[0] != NULL)
		{
			glEnable(GL_COLOR_MATERIAL);
		}
		else
		{
			glDisable(GL_COLOR_MATERIAL);
		}

		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch (face->mNumIndices)
			{
			case 1: face_mode = GL_POINTS; break;
			case 2: face_mode = GL_LINES; break;
			case 3: face_mode = GL_TRIANGLES; break;
			default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);

			for (i = 0; i < face->mNumIndices; i++)		// go through all vertices in face
			{
				int vertexIndex = face->mIndices[i];	// get group index for current index
				if (mesh->mColors[0] != NULL)
					Color4f(&mesh->mColors[0][vertexIndex]);
				if (mesh->mNormals != NULL)

					if (mesh->HasTextureCoords(0))		//HasTextureCoords(texture_coordinates_set)
					{
						glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y); //mTextureCoords[channel][vertex]
					}
				glNormal3fv(&mesh->mNormals[vertexIndex].x);
				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
			glEnd();
		}
	}
	glPopMatrix();
}


void init()
{
	glClearColor(0.0, 0.0, 0.0, 0);
	glColor3f(1.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(-1200, 1200, -700, 700);                   //<-----CHANGE THIS TO GET EXTRA SPACE
	glMatrixMode(GL_MODELVIEW);
}

void backButtonMenu() {
	glDisable(GL_LIGHTING);
	if (mouseX <= -500 && mouseX >= -1050 && mouseY >= -600 && mouseY <= -500) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			if (statoGioco == 4) {
				initi = 0;
				if (musicON)
					PlaySound(TEXT("ToyStorySong.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
			}
			statoGioco = 1;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(0, 1, 0);
	displayRasterText(-1000, -550, 0, "Torna al menu'");
}

void backButtonPause() {
	glDisable(GL_LIGHTING);
	if (mouseX <= -600 && mouseX >= -1050 && mouseY >= -600 && mouseY <= -500) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			statoGioco = 7;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(0, 1, 0);
	displayRasterText(-1000, -550, 0, "Indietro");
}

void drawIntro()
{
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLineWidth(50);
	glColor3f(0.0, 1.0, 0.0);
	displayRasterText(-200, 400, 0.0, "ZURG ATTACK");
	glLineWidth(10);
	glColor3f(0.8, 1.0, 0.8);
	displayRasterText(-230 + Width * 50 / initWIDTH - 50, 300, 0.0, "basato su Toy Story");
	glColor3f(0.0, 0.5, 1.0);
	displayRasterText(-1000, -120, 0.0, "Politecnico di Torino - a.a. 2021/22");
	displayRasterText(-1000, -200, 0.0, "Ideato e realizzato da");
	displayRasterText(300, -200, 0.0, "Docenti");
	glColor3f(1.0, 1.0, 0.0);
	displayRasterText(-850, -300, 0.0, "Federico Mafrici");
	displayRasterText(-850, -380, 0.0, "Kevin Mascitti");
	displayRasterText(450, -300, 0.0, "Alberto Cannavo'");
	displayRasterText(450, -380, 0.0, "Fabrizio Lamberti");
	glColor3f(1.0, 1.0, 1.0);
	displayRasterText(-420 + Width * 200 / initWIDTH - 200, -580, 0.0, "Premi SPAZIO per iniziare il gioco");
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// render Object
	glEnable(GL_DEPTH_TEST);
	float tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;
	glRotatef(-45, 0.f, 1.f, 0.f);
	//	glRotatef(180, 1.f, 0.f, 0.f);
		//glScalef(tmp, tmp, tmp);
	glScalef(tmp, tmp, tmp);
	glTranslatef(1000, 0, 100);
	recursive_render(scene, scene->mRootNode->mChildren[0], 1.0);
	glTranslatef(-1000, 0, -100);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	init();
}

void drawSchermataIniziale()
{
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLineWidth(20);
	glColor3f(0, 1, 0);
	glBegin(GL_LINE_LOOP);               // Bordo
	glVertex2f(-750, -600);
	glVertex2f(-750, 550);
	glVertex2f(750, 550);
	glVertex2f(750, -600);
	glEnd();

	glLineWidth(1);

	glColor3f(0, 0.4, 1.0);
	glBegin(GL_POLYGON);				// AVVIA PARTITA 
	glVertex2f(-400, 300);
	glVertex2f(-400, 400);
	glVertex2f(400, 400);
	glVertex2f(400, 300);
	glEnd();

	glBegin(GL_POLYGON);				// VISUALIZZA RECORD
	glVertex2f(-400, 200);
	glVertex2f(-400, 100);
	glVertex2f(400, 100);
	glVertex2f(400, 200);
	glEnd();

	glBegin(GL_POLYGON);				// VISUALIZZA COMANDI
	glVertex2f(-400, -100);
	glVertex2f(-400, 0);
	glVertex2f(400, 0);
	glVertex2f(400, -100);
	glEnd();

	glBegin(GL_POLYGON);				// IMPOSTAZIONI
	glVertex2f(-400, -200);
	glVertex2f(-400, -300);
	glVertex2f(400, -300);
	glVertex2f(400, -200);
	glEnd();

	int sign, smaller;
	if (mouseY > 0)
		sign = -1;
	else
		sign = 1;
	if (Height < initHEIGHT)
		smaller = 1;
	else
		smaller = 0;

	if (mouseX >= -400 && mouseX <= 400 && mouseY >= 300 && mouseY <= 400) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			initi = 0;
			statoGioco = 2;
			numeroPartite++;
			if (musicON)
				PlaySound(TEXT("ZurgTheme.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);

	displayRasterText(-130 + Width * 80 / initWIDTH - 80, 330, 0, "Avvia partita");

	if (mouseX >= -400 && mouseX <= 400 && mouseY >= 100 && mouseY <= 200) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			statoGioco = 5;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	displayRasterText(-250 + Width * 160 / initWIDTH - 160, 130, 0, "Visualizza i tuoi record");

	if (mouseX >= -400 && mouseX <= 400 && mouseY >= -100 && mouseY <= 0) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			statoGioco = 8;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	displayRasterText(-240 + Width * 150 / initWIDTH - 150, -70, 0, "Visualizza i comandi");

	if (mouseX >= -400 && mouseX <= 400 && mouseY >= -300 && mouseY <= -200) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			statoGioco = 6;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	displayRasterText(-140 + Width * 90 / initWIDTH - 90, -270, 0, "Impostazioni");

	if (mouseX >= -230 && mouseX <= 230 && mouseY >= -550 && mouseY <= -450) {
		glColor3f(1, 0, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			mButtonPressed = false;
			exit(0);
		}
	}
	else
		glColor3f(1, 1, 1);
	displayRasterText(-160 + Width * 110 / initWIDTH - 110, -520, 0, "Esci dal gioco");

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// render Object
	glEnable(GL_DEPTH_TEST);
	float tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;
	glRotatef(180.f, 0.f, 1.f, 0.f);
	glRotatef(-45.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-1500, 800, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[5], 1.0);
	glTranslatef(+1500, -800, -1000);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	//glRotatef(0.f, 0.f, 1.f, 0.f);
	glRotatef(-45.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-1600, 600, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[2], 1.0);
	glTranslatef(+1600, -600, -1000);


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	
	glDisable(GL_DEPTH_TEST);
	init();
}

void gameOver()
{
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glLoadIdentity();
	glColor3f(1, 0, 0);
	displayRasterText(-190 + Width * 80 / initWIDTH - 80, 300, 0, "GAME OVER");
	glColor3f(0, 1, 0);
	if (Punteggio > 0) {
		if (numeroPartite == 1 || (record.empty() == FALSE && Punteggio > record.begin()->first))
			displayRasterText(-200 + Width * 110 / initWIDTH - 110, 100, 0, "Nuovo record!");
		auto now = std::chrono::system_clock::now();
		std::time_t end_time = std::chrono::system_clock::to_time_t(now);
		record.insert({ Punteggio, std::ctime(&end_time) });
	}
	std::string str = "Hai totalizzato " + std::to_string(Punteggio) + " punti!";
	char* chars = new char[str.length() + 1];
	strcpy(chars, str.c_str());
	displayRasterText(-300 + Width * 150 / initWIDTH - 150, 0, 0, chars);
	glColor3f(1, 1, 1);
	displayRasterText(-550 + Width * 270 / initWIDTH - 270, -300, 0, "Premi SPAZIO per iniziare una nuova partita");

	backButtonMenu();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// render Object
	glEnable(GL_DEPTH_TEST);
	float tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;
	glRotatef(-45, 0.f, 1.f, 0.f);
	//	glRotatef(180, 1.f, 0.f, 0.f);
		//glScalef(tmp, tmp, tmp);
	glScalef(tmp, tmp, tmp);
	glTranslatef(1000, 0, 100);
	recursive_render(scene, scene->mRootNode->mChildren[0], 1.0);
	glTranslatef(-1000, 0, -100);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	init();
}


void drawRecord() {
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (record.empty() == TRUE) {
		glColor3f(0, 0, 1);
		displayRasterText(-500 + Width * 200 / initWIDTH - 200, 350, 0, "Non sono mai stati effettuati dei record.");
	}
	else {
		glColor3f(0, 1, 0);
		displayRasterText(-120 + Width * 40 / initWIDTH - 40, 450, 0, "RECORD");
		displayRasterText(-800 + Width * 100 / initWIDTH - 100, 350, 0, "Punteggio");
		displayRasterText(-300 + Width * 30 / initWIDTH - 30, 350, 0, "Data");
		glColor3f(0, 0.4, 1);
		std::string str = "";
		int i = 0;
		std::map<int, std::string>::iterator it;
		for (it = record.begin(); it != record.end(); ++it) {
			if (i == MAX)
				break;
			str = std::to_string(it->first);
			char* chars = new char[str.length() + 1];
			strcpy(chars, str.c_str());
			displayRasterText(-700 + Width * 20 / initWIDTH - 20, 280 - i * 70, 0, chars);

			str = it->second;
			chars = new char[str.length() + 1];
			strcpy(chars, str.c_str());
			displayRasterText(-300 + Width * 30 / initWIDTH - 30, 280 - i * 70, 0, chars);
			i++;
		}
	}
	backButtonMenu();


	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// render Object
	glEnable(GL_DEPTH_TEST);
	float tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;
	glRotatef(180.f, 0.f, 1.f, 0.f);
	glRotatef(-45.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-1500, 800, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[8], 1.0); 
	glTranslatef(+1500, -800, -1000);
	//glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	glRotatef(160.f, 0.f, 1.f, 0.f);
	glRotatef(-45.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-1500,0, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[8], 1.0);
	glTranslatef(+1500, 0, -1000);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	//glRotatef(0.f, 0.f, 1.f, 0.f);
	glRotatef(-45.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-2000, 600, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[9], 1.0);
	glTranslatef(+2000, -600, -1000);
	
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	//glRotatef(0.f, 0.f, 1.f, 0.f);
	glRotatef(-25.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-2000, 200, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[9], 1.0);
	glTranslatef(+2000, -200, -1000);


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	
	glDisable(GL_DEPTH_TEST);
	init();
}

void drawComandi()
{
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(0, 1, 0);
	displayRasterText(-370, 450, 0, "COMANDI DI GIOCO");
	glColor3f(1, 1, 1);
	displayRasterText(-800, 300, 0, "A per muovere l'astronave a sinistra.");
	displayRasterText(-800, 200, 0, "D per muovere l'astronave a destra.");
	displayRasterText(-800, 100, 0, "SPAZIO per usare un superpotere.");
	displayRasterText(-800, 0, 0, "CLICK per cambiare visuale.");
	displayRasterText(-800, -100, 0, "L'obiettivo del gioco e' evitare gli ostacoli e accumulare punti");
	displayRasterText(-800, -170, 0, "raccogliendo monete e utilizzando i superpoteri contenuti nei cubi.");
	displayRasterText(-800, -240, 0, "I cuori rigenerano 25 punti vita.");
	displayRasterText(-800, -310, 0, "Se la vita arriva a 0, la partita termina.");
	backButtonMenu();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// render Object
	glEnable(GL_DEPTH_TEST);
	float tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;
	glRotatef(180.f, 0.f, 1.f, 0.f);
	glRotatef(-45.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-1500, 800, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[5], 1.0);
	glTranslatef(+1500, -800, -1000);
	//glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//const double aspectRatio = (float)Width / Height, fieldOfView = 45.0;
	gluPerspective(fieldOfView, aspectRatio, 1.0, 1000.0);  // Znear and Zfar 
	gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	//glRotatef(0.f, 0.f, 1.f, 0.f);
	glRotatef(0.f, 1.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(-1800, 1000, 1000);
	recursive_render(scene, scene->mRootNode->mChildren[1], 1.0);
	glTranslatef(-1800, -1000, -1000);


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	init();
}

void drawComandiPause()
{
	glDisable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(0, 1, 0);
	displayRasterText(-370, 450, 0, "COMANDI DI GIOCO");
	glColor3f(1, 1, 1);
	displayRasterText(-800, 300, 0, "A per muovere l'astronave a sinistra.");
	displayRasterText(-800, 200, 0, "D per muovere l'astronave a destra.");
	displayRasterText(-800, 100, 0, "SPAZIO per usare un superpotere.");
	displayRasterText(-800, 0, 0, "CLICK per cambiare visuale.");
	displayRasterText(-800, -100, 0, "L'obiettivo del gioco e' evitare gli ostacoli e accumulare punti");
	displayRasterText(-800, -170, 0, "raccogliendo monete e utilizzando i superpoteri contenuti nei cubi.");
	displayRasterText(-800, -240, 0, "I cuori rigenerano 25 punti vita.");
	displayRasterText(-800, -310, 0, "Se la vita arriva a 0, la partita termina.");
	backButtonPause();
}

void drawSettings() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glLineWidth(20);

	glLineWidth(1);

	glColor3f(0, 0.5, 1);
	glBegin(GL_POLYGON);				// FACILE
	glVertex2f(-500, 100);
	glVertex2f(-500, 200);
	glVertex2f(-200, 200);
	glVertex2f(-200, 100);
	glEnd();

	glBegin(GL_POLYGON);				// NORMALE
	glVertex2f(-150, 100);
	glVertex2f(-150, 200);
	glVertex2f(150, 200);
	glVertex2f(150, 100);
	glEnd();


	glBegin(GL_POLYGON);				// DIFFICILE
	glVertex2f(200, 100);
	glVertex2f(200, 200);
	glVertex2f(500, 200);
	glVertex2f(500, 100);
	glEnd();

	glBegin(GL_POLYGON);				// Musica OFF
	glVertex2f(-400, -50);
	glVertex2f(-400, -150);
	glVertex2f(-100, -150);
	glVertex2f(-100, -50);
	glEnd();

	glBegin(GL_POLYGON);				// Musica ON
	glVertex2f(100, -50);
	glVertex2f(100, -150);
	glVertex2f(400, -150);
	glVertex2f(400, -50);
	glEnd();

	glBegin(GL_POLYGON);				// Effetti OFF
	glVertex2f(-400, -300);
	glVertex2f(-400, -400);
	glVertex2f(-100, -400);
	glVertex2f(-100, -300);
	glEnd();

	glBegin(GL_POLYGON);				// Effetti ON
	glVertex2f(100, -300);
	glVertex2f(100, -400);
	glVertex2f(400, -400);
	glVertex2f(400, -300);
	glEnd();

	glColor3f(0, 1, 0);
	displayRasterText(-230 + Width * 130 / initWIDTH - 130, 450, 0, "IMPOSTAZIONI");

	glColor3f(0, 0.5, 1);
	displayRasterText(-240 + Width * 160 / initWIDTH - 160, 250, 0, "Scegli la difficolta'");

	if (mouseX >= -500 && mouseX <= -200 && mouseY >= 100 && mouseY <= 200) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON && livello != 1)
				effects.Play(CHOOSE);
			livello = 1;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	if (livello == 1)
		glColor3f(0, 1, 0);
	displayRasterText(-430 + Width * 60 / initWIDTH - 60, 130, 0, "Facile");

	if (mouseX >= -150 && mouseX <= 150 && mouseY >= 100 && mouseY <= 200) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON && livello != 2)
				effects.Play(CHOOSE);
			livello = 2;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	if (livello == 2)
		glColor3f(0, 1, 0);
	displayRasterText(-110 + Width * 70 / initWIDTH - 70, 130, 0, "Normale");

	if (mouseX >= 200 && mouseX <= 500 && mouseY >= 100 && mouseY <= 200) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON && livello != 3)
				effects.Play(CHOOSE);
			livello = 3;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	if (livello == 3)
		glColor3f(0, 1, 0);
	displayRasterText(250 + Width * 70 / initWIDTH - 70, 130, 0, "Difficile");

	glColor3f(0, 0.5, 1);
	displayRasterText(-100 + Width * 60 / initWIDTH - 60, 0, 0, "Musica");

	if (mouseX >= -400 && mouseX <= -100 && mouseY >= -150 && mouseY <= -50) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON && musicON != 0)
				effects.Play(CHOOSE);
			PlaySound(0, 0, 0);
			musicON = 0;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	if (musicON == 0)
		glColor3f(0, 1, 0);
	displayRasterText(-320 + Width * 40 / initWIDTH - 40, -120, 0, "OFF");

	if (mouseX >= 100 && mouseX <= 400 && mouseY >= -150 && mouseY <= -50) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON && musicON != 1)
				effects.Play(CHOOSE);
			if (musicON != 1)
				PlaySound(TEXT("ToyStorySong.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
			musicON = 1;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	if (musicON == 1)
		glColor3f(0, 1, 0);
	displayRasterText(200 + Width * 40 / initWIDTH - 40, -120, 0, "ON");

	glColor3f(0, 0.5, 1);
	displayRasterText(-100 + Width * 60 / initWIDTH - 60, -250, 0.4, "Effetti");

	if (mouseX >= -400 && mouseX <= -100 && mouseY >= -400 && mouseY <= -300) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			effectON = 0;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	if (effectON == 0)
		glColor3f(0, 1, 0);
	displayRasterText(-320, -370, 0.4, "OFF");

	if (mouseX >= 100 && mouseX <= 400 && mouseY >= -400 && mouseY <= -300) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON != 1)
				effects.Play(CHOOSE);
			effectON = 1;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	if (effectON == 1)
		glColor3f(0, 1, 0);
	displayRasterText(200, -370, 0.4, "ON");

	backButtonMenu();
}

void drawPause()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glLineWidth(1);

	glColor3f(0, 0.5, 1);
	glBegin(GL_POLYGON);				// RIPRENDI PARTITA 
	glVertex2f(-400, 100);
	glVertex2f(-400, 200);
	glVertex2f(400, 200);
	glVertex2f(400, 100);
	glEnd();

	glBegin(GL_POLYGON);				// VISUALIZZA COMANDI
	glVertex2f(-400, 0);
	glVertex2f(-400, -100);
	glVertex2f(400, -100);
	glVertex2f(400, 0);
	glEnd();

	glBegin(GL_POLYGON);				// TORNA AL MENU
	glVertex2f(-400, -200);
	glVertex2f(-400, -300);
	glVertex2f(400, -300);
	glVertex2f(400, -200);
	glEnd();

	glColor3f(0, 1, 0);
	displayRasterText(-250 + Width * 150 / initWIDTH - 150, 300, 0, "GIOCO IN PAUSA");

	if (mouseX >= -400 && mouseX <= 400 && mouseY >= 100 && mouseY <= 200) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			statoGioco = 2;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	displayRasterText(-200 + Width * 130 / initWIDTH - 130, 130, 0, "Riprendi partita");

	if (mouseX >= -400 && mouseX <= 400 && mouseY >= -100 && mouseY <= 0) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			statoGioco = 9;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	displayRasterText(-250 + Width * 150 / initWIDTH - 150, -70, 0, "Visualizza i comandi");


	if (mouseX >= -400 && mouseX <= 400 && mouseY >= -300 && mouseY <= -200) {
		glColor3f(1, 1, 0);
		if (mButtonPressed) {
			if (effectON)
				effects.Play(CHOOSE);
			statoGioco = 1;
			initi = 0;
			if (musicON)
				PlaySound(TEXT("ToyStorySong.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
			mButtonPressed = false;
		}
	}
	else
		glColor3f(1, 1, 1);
	displayRasterText(-350 + Width * 270 / initWIDTH - 270, -270, 0, "Torna al menu' principale");
}


void abbassaTutto(int spostZ)
{
	for (int i = 0; i < objectCount ;i++)
	{
		if (oggetti[i].ID != -1) 
		{
			oggetti[i].Z += spostZ;
			oggetti[i].Vmin.z += spostZ;
			oggetti[i].Vmax.z += spostZ;
		}
		
		if (oggetti[i].Z > astronave.Z + 300)
			oggetti[i].ID = -1;
	}
	if (laser.ID != -1)
	{
		laser.Z += spostZ;
		laser.Vmin.z += spostZ;
		laser.Vmax.z += spostZ;
	}
	if (laser.Z > astronave.Z + 300)
		laser.ID = -1;
}

GLint prev_time2 = 0.0;

void do_motion(void)
{
	static GLint prev_time = 0;

	int time = glutGet(GLUT_ELAPSED_TIME);

	if (time - prev_time2 >= 1000*difficolta) {
		abbassaTutto(400);

		prev_time2 = time;
	}
	prev_time = time;
}

int setBoss() 
{
	boss = 1;
	return 1;
}

void stopBoss(int null) {
	boss = 0;
}

void stopDisplayBoss(int numeroPartita)
{
	if(numeroPartite==numeroPartita)
	displayBoss = 0;
}

void generaLaser(int randompos) 
{
	struct aiMatrix4x4 trafo;
	laser.ID = 4;
	laser.X = -scene_center.x + posizioni[randompos];
	laser.height = astronave.height;
	laser.Z = scene_center.z - InitialZ;
	laser.Vmin.x = laser.Vmin.y = laser.Vmin.z = 1e10f;
	laser.Vmax.x = laser.Vmax.y = laser.Vmax.z = -1e10f;
	laser.collisione = 0;
	aiIdentityMatrix4(&trafo);
	get_bounding_box_for_node(scene->mRootNode->mChildren[laser.ID], &laser.Vmin, &laser.Vmax, &trafo);
	
	laser.Vmin.x = laser.Vmin.x + laser.X;
	laser.Vmax.x = laser.Vmax.x + laser.X;

	laser.Vmin.y = laser.Vmin.y - InitialY;
	laser.Vmax.y = laser.Vmax.y - InitialY;

	laser.Vmin.z = laser.Vmin.z - InitialZ;
	laser.Vmax.z = laser.Vmax.z - InitialZ;	
}

int  generaBoss()
{
	setBoss();
	
	int  randompos = 3 + (rand() % 7);
	struct aiMatrix4x4 trafo;
	effects.Play(ZURG);
	zurg.X = -scene_center.x + posizioni[randompos];
	
	switch (randompos)
	{
	case 0:
		zurg.X += 1800;
		break;
	case 1:
		zurg.X += 1600;
		break;
	case 2:
		zurg.X +=1300;
		break;
	case 3:
		zurg.X += 1050;
		break;
	case 4:
		zurg.X += 800;
		break;
	case 5:
		zurg.X += 550;
		break;
	case 6:
		zurg.X +=270;
		break;
	case 7:
		zurg.X+=50;
		break;
	case 8:
		zurg.X -= 200;
		break;
	case 9:
		zurg.X -= 450;
		break;

	
	default:
		break;
	}

	
	// NON MODIFICARE VALORI Y E Z!!!
	zurg.height = - InitialY + 770;
	zurg.Z = scene_center.z - InitialZ + 1500;

	aiIdentityMatrix4(&trafo);
	get_bounding_box_for_node(scene->mRootNode->mChildren[zurg.ID], &oggetti[objectCount].Vmin, &oggetti[objectCount].Vmax, &trafo);
	oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.x + posizioni[randompos];
	oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.x + posizioni[randompos];

	oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.y - InitialY;
	oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.y - InitialY;

	oggetti[objectCount].Vmin.z = oggetti[objectCount].Vmin.z - InitialZ;
	oggetti[objectCount].Vmax.z = oggetti[objectCount].Vmax.z - InitialZ;

	generaLaser(randompos);

	return randompos;
}

// se segno=2 allora  positivo se uguale 1 negativo serve per aumentare la dispersione
int generaOggetto(float x, float y, float z)
{
	int ID = -1; // randomizer tra 0 e 5 
	contatoreMonete++;
	contatoreCubo++;

	if (contatoreMonete > 3)
	{
		contatoreMonete = 0;
		ID = 8; // id moneta ne voglia una ogni 3 oggetti per non essere troppo frequente o troppo poco 
		int randompos = (rand() % 14);
		oggetti[objectCount].X = -x + posizioni[randompos];
		oggetti[objectCount].ID = ID;
		oggetti[objectCount].height = - InitialY;
		oggetti[objectCount].Z = z - InitialZ;
		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.z = 1e10f;
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.z = -1e10f;
		struct aiMatrix4x4 trafo;
		aiIdentityMatrix4(&trafo);
		get_bounding_box_for_node(scene->mRootNode->mChildren[ID], &oggetti[objectCount].Vmin, &oggetti[objectCount].Vmax, &trafo);
		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.x + posizioni[randompos];
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.x + posizioni[randompos];

		oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.y - InitialY;
		oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.y - InitialY;

		oggetti[objectCount].Vmin.z = oggetti[objectCount].Vmin.z - InitialZ;
		oggetti[objectCount].Vmax.z = oggetti[objectCount].Vmax.z - InitialZ;

		objectCount++;
		return ID;
	}

	if (contatoreCubo > 5)
	{
		contatoreCubo = 0;
		ID = 9 + (rand() % 2); // randomizer che genera o un cuore o un power up  non deve essere frequente quindi lo regolo ogni N oggetti spawnati

		int  randompos = (rand() % 14);
		oggetti[objectCount].X = -x + posizioni[randompos];
		oggetti[objectCount].ID = ID;
		oggetti[objectCount].height = y - InitialY;
		oggetti[objectCount].Z = z - InitialZ;
		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.z = 1e10f;
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.z = -1e10f;
		struct aiMatrix4x4 trafo;
		aiIdentityMatrix4(&trafo);
		get_bounding_box_for_node(scene->mRootNode->mChildren[ID], &oggetti[objectCount].Vmin, &oggetti[objectCount].Vmax, &trafo);
		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.x + posizioni[randompos];
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.x + posizioni[randompos];

		oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.y - InitialY;
		oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.y - InitialY;

		oggetti[objectCount].Vmin.z = oggetti[objectCount].Vmin.z - InitialZ;
		oggetti[objectCount].Vmax.z = oggetti[objectCount].Vmax.z - InitialZ;

		objectCount++;

		return ID;
	}
	else
	{
		// generatore di ostacoli molto pi˘ frequente degli altri 
		int randobj = rand() % 6;
		if (randobj >= 4)
			ID = 1 + (rand() % 3);
		else
			ID = 6;
		struct aiMatrix4x4 trafo;
		int  randompos1 = (rand() % 14);
		// genero la posizione x randomica e l'altezza di partenza
		oggetti[objectCount].X = -x + posizioni[randompos1];
		oggetti[objectCount].ID = ID;
		oggetti[objectCount].height = -InitialY;
		oggetti[objectCount].Z = z - InitialZ;

		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.z = 1e10f;
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.z = -1e10f;
		aiIdentityMatrix4(&trafo);
		get_bounding_box_for_node(scene->mRootNode->mChildren[ID], &oggetti[objectCount].Vmin, &oggetti[objectCount].Vmax, &trafo);
		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.x + posizioni[randompos1];
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.x + posizioni[randompos1];

		oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.y - InitialY;
		oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.y - InitialY;

		oggetti[objectCount].Vmin.z = oggetti[objectCount].Vmin.z - InitialZ;
		oggetti[objectCount].Vmax.z = oggetti[objectCount].Vmax.z - InitialZ;

		objectCount++;

		int ID2 = -1;
		randobj = rand() % 6;
		if (randobj >= 4)
			ID2 = 1 + (rand() % 3);
		else
			ID2 = 6;

		while (ID == ID2)
		{
			int randobj = rand() % 6;
			if (randobj >= 4)
				ID2 = 1 + (rand() % 3);
			else
				ID2 = 6;
		}
		oggetti[objectCount].ID = ID2;
		oggetti[objectCount].height = -InitialY;
		oggetti[objectCount].Z = z - InitialZ;

		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.z = 1e10f;
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.z = -1e10f;

		aiIdentityMatrix4(&trafo);
		get_bounding_box_for_node(scene->mRootNode->mChildren[ID2], &oggetti[objectCount].Vmin, &oggetti[objectCount].Vmax, &trafo);
		// calcolate le bounding box queste poi vengono traslate del valore preso tramite randomizer con l'accorgimento di non poter ottenere la stessa x due volte 
		int randompos2 = (rand() % 14);
		while (randompos1 == randompos2)
			randompos2 = (rand() % 14);

		oggetti[objectCount].X = -x + posizioni[randompos2];
		oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.x + posizioni[randompos2];
		oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.x + posizioni[randompos2];

		oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.y - InitialY;
		oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.y - InitialY;

		oggetti[objectCount].Vmin.z = oggetti[objectCount].Vmin.z - InitialZ;
		oggetti[objectCount].Vmax.z = oggetti[objectCount].Vmax.z - InitialZ;

		objectCount++;
		return ID;
	}
	return -1;
}

int generaOggettoBoss(float x, float y, float z,int posZurg)
{
	int ID = -1;
	int  randompos1 = (rand() % 14);
	// generatore di ostacoli molto più frequente degli altri 
	
	int randobj = rand() % 6;
	if (randobj >= 4)
		ID = 1 + (rand() % 3);
	else
		ID = 6;
	struct aiMatrix4x4 trafo;
	
	while(randompos1==posZurg)
	randompos1 = (rand() % 14);
	// genero la posizione x randomica e l'altezza di partenza
	oggetti[objectCount].X = -x + posizioni[randompos1];
	oggetti[objectCount].ID = ID;
	oggetti[objectCount].height = -InitialY;
	oggetti[objectCount].Z = z - InitialZ;
	
	oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.z = 1e10f;
	oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.z = -1e10f;
		
	aiIdentityMatrix4(&trafo);
	get_bounding_box_for_node(scene->mRootNode->mChildren[ID], &oggetti[objectCount].Vmin, &oggetti[objectCount].Vmax, &trafo);
	
	oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.x + posizioni[randompos1];
	oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.x + posizioni[randompos1];

	oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.y - InitialY;
	oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.y - InitialY;

	oggetti[objectCount].Vmin.z = oggetti[objectCount].Vmin.z - InitialZ;
	oggetti[objectCount].Vmax.z = oggetti[objectCount].Vmax.z - InitialZ;

	objectCount++;
	int ID2 = -1;
	randobj = rand() % 6;
	if (randobj >= 4)
		ID2 = 1 + (rand() % 3);
	else
		ID2 = 6;

	while (ID == ID2)
	{
		int randobj = rand() % 6;
		if (randobj >= 4)
			ID2 = 1 + (rand() % 3);
		else
			ID2 = 6;
	}

	oggetti[objectCount].ID = ID2;
	oggetti[objectCount].height = -InitialY;
	oggetti[objectCount].Z = z - InitialZ;
	oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.z = 1e10f;
	oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.z = -1e10f;
	// molto importante i vertici max e min vanno sempre resettati perchè sennà con la bomba poi io tengo le bounding box precedenti che potrebbero romper l'algoritmo
	aiIdentityMatrix4(&trafo);
	get_bounding_box_for_node(scene->mRootNode->mChildren[ID], &oggetti[objectCount].Vmin, &oggetti[objectCount].Vmax, &trafo);
	// calcolate le bounding box queste poi vengono traslate del valore preso tramite randomizer con l'accorgimento di non poter ottenere la stessa x due volte 

	int randompos2 = (rand() % 14);
	int tmp = 0;
	while (randompos1 == randompos2 || randompos2==posZurg)
	{
		tmp = (rand() % 14);
		if (tmp != posZurg)
			randompos2 = tmp;
	}
		
	oggetti[objectCount].X = -x + posizioni[randompos2];
	oggetti[objectCount].Vmin.x = oggetti[objectCount].Vmin.x + posizioni[randompos2];
	oggetti[objectCount].Vmax.x = oggetti[objectCount].Vmax.x + posizioni[randompos2];

	oggetti[objectCount].Vmin.y = oggetti[objectCount].Vmin.y - InitialY;
	oggetti[objectCount].Vmax.y = oggetti[objectCount].Vmax.y - InitialY;

	oggetti[objectCount].Vmin.z = oggetti[objectCount].Vmin.z - InitialZ;
	oggetti[objectCount].Vmax.z = oggetti[objectCount].Vmax.z - InitialZ;

	objectCount++;
	return ID;
}

int checkLaserCollision()
{
	if (laser.Vmax.z  > astronave.Vmin.z && laser.Vmax.z < astronave.Vmax.z)
		if(laser.Vmax.x>astronave.Vmin.x && laser.Vmax.x<astronave.Vmax.x)
				return 1;
	if (laser.Vmax.z > astronave.Vmin.z && laser.Vmax.z < astronave.Vmax.z)
		if (laser.Vmin.x > astronave.Vmin.x && laser.Vmin.x < astronave.Vmax.x)
			return 1;
	return 0;
 }

int checkCollision(int ID)
{
	if (oggetti[ID].Vmax.z > astronave.Vmin.z && oggetti[ID].Vmax.z < astronave.Vmax.z)
		if (oggetti[ID].Vmax.x > astronave.Vmin.x && oggetti[ID].Vmax.x < astronave.Vmax.x)
			return 1;
	if (oggetti[ID].Vmax.z > astronave.Vmin.z && oggetti[ID].Vmax.z < astronave.Vmax.z)
		if (oggetti[ID].Vmin.x > astronave.Vmin.x && oggetti[ID].Vmin.x < astronave.Vmax.x)
			return 1;
	return 0;
}

void InitGame() {
	// inizializzazione
	astronave.height = scene_center.y - InitialY;
	astronave.X = scene_center.x;
	astronave.Z = -scene_center.z + AstronaveZ;
	HP = initHP;
	Punteggio = initPUNT;
	punteggio = std::to_string(initPUNT);
	difficoltaZurg = STDZurg;
	difficolta = STDDiff;
	visuale = 1;
	//reset movimento e oggetti
	for (int i = 0; i < objectCount; i++)
	{
		oggetti[i].ID = -1;
		oggetti[i].collisione = 0;
		oggetti[i].Vmin.x = oggetti[i].Vmin.y = oggetti[i].Vmin.z = 1e10f;
		oggetti[i].Vmax.x = oggetti[i].Vmax.y = oggetti[i].Vmax.z = -1e10f;
	}
	contatoreMonete = 0;
	contatoreCubo = 0;
	objectCount = 0;
	change = 0;
	powerUp = "";
	powerUpId = -1;
	astronave.ID = 0;
	laser.ID = -1;
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
	astronave.Vmax.x = astronave.Vmax.y = astronave.Vmax.z = -1e10f;
	astronave.Vmin.x = astronave.Vmin.y = astronave.Vmin.z = 1e10f;
	get_bounding_box_for_node(scene->mRootNode->mChildren[5], &astronave.Vmin, &astronave.Vmax, &trafo);

	astronave.Vmax.y = astronave.Vmax.y - InitialY;
	astronave.Vmin.y = astronave.Vmin.y - InitialY;

	astronave.Vmin.z = astronave.Vmin.z + scene_center.z + AstronaveZ;
	astronave.Vmax.z = astronave.Vmax.z + scene_center.z + AstronaveZ;

	//reset boss
	zurg.ID = 0;
	boss = 0;
	firstDisplay = 1;
	displayBoss = 0;
	initi++;
	primaChiamataOggetto = 1;
	primaChiamataBoss = 1;
	abilitaGenerazioneBoss = 0;
	abilitaGenerazioneOggetto = 1;
	return;
}
void abilitaOggetto(int null)
{
	abilitaGenerazioneOggetto = 1;
}
void abilitaBoss(int null)
{
	abilitaGenerazioneBoss = 1;
}

void drawGame(void)
{
	float tmp;
	static GLint prev_time_object = 0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	int ID = -1;
	int time=-1;
	char* str = new char[100];
	tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;
	if (mButtonPressed == true)
		glitch++;
	if (mButtonPressed == true && glitch == 1)
		visuale++;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (visuale % 3 == 0)
		gluLookAt(0.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	else if (visuale % 3 == 1)
		gluLookAt(0.f, 2.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	else if (visuale % 3 == 2)
		gluLookAt(0.f, 3.3f, 0.5f, 0.f, 0.f, -0.3f, 0.f, 1.f, 0.f);
	glTranslatef(0, 0, 0);

	// scale the whole asset to fit into our view frustum 
	tmp = scene_max.x - scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y, tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z, tmp);
	tmp = 1.f / tmp;

	// sfondo
	glMatrixMode(GL_PROJECTION_MATRIX);
	glLoadIdentity();
	glTranslatef(0, 0, -scene_center.z - 1100);
	glScalef(2.f, 3.5f, 2.f); 
	recursive_render(scene, scene->mRootNode->mChildren[7], 1.0);

	glPopMatrix();

	glMatrixMode(GL_PROJECTION_MATRIX);
	glLoadIdentity();
	glPopMatrix();

	// center the model
	// float new_center = -scene_center.y - spostZ;
	// Inizializzo le coordinate dell'astronave calcolo la sua bounding box e traslo i vmin e vmax
	if (initi == 0)
	{
		InitGame();
		initi++;
	}

	// now begin at the root node of the imported data and traverse
	// the scenegraph by multiplying subsequent local transforms
	// together on GL's matrix stack.
	
	for (int i = 0; i < objectCount; i++)
	{
		if (!oggetti[i].collisione && oggetti[i].ID != -1)
		{
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			if (visuale % 3 == 0)
				gluLookAt(0.f, -0.5, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
			else if (visuale % 3 == 1)
				gluLookAt(0.f, 2.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
			else if (visuale % 3 == 2)
				gluLookAt(0.f, 3.3f, 0.5f, 0.f, 0.f, -0.3f, 0.f, 1.f, 0.f);
			glRotatef(0.f, 0.f, 1.f, 0.f);
			glScalef(tmp / 2, tmp / 2, tmp / 2);

			glTranslatef(oggetti[i].X, oggetti[i].height, oggetti[i].Z);
			recursive_render(scene, scene->mRootNode->mChildren[oggetti[i].ID], 1.0); //stampo gli oggetti con gli ID corrispondenti
			glTranslatef(-oggetti[i].X, -oggetti[i].height, -oggetti[i].Z);
			glPopMatrix();

			if (checkCollision(i) && !oggetti[i].collisione)
			{
				if (checkCollision(i))
					oggetti[i].collisione = 1;
				gameManager(oggetti[i].ID);
			}
		}
	}
	
	// decisione dell'oggetto da stampare: 
	 time = glutGet(GLUT_ELAPSED_TIME);
	 if (time - prev_time3 > 30000 * difficolta && abilitaGenerazioneBoss && Punteggio > 0)
	 {
		 int IndicePosZurg = generaBoss();
		 generaOggettoBoss(scene_center.x, scene_center.y, scene_center.z, IndicePosZurg);
		 prev_time3 = time;
		 displayBoss = 1;
		 firstDisplay = 1;
		 stopBoss(0);
		 abilitaGenerazioneOggetto = 0;
		 primaChiamataOggetto = 1;
		 
	 }
	
	if (time - prev_time_object > 2500 * difficolta && abilitaGenerazioneOggetto)
	{
		ID = generaOggetto(scene_center.x, scene_center.y, scene_center.z);
		prev_time_object = time;
		abilitaGenerazioneBoss = 0;
		primaChiamataBoss = 1;
	}
	
	if (displayBoss)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (visuale % 3 == 0)
			gluLookAt(0.f, -0.5, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
		else if (visuale % 3 == 1)
			gluLookAt(0.f, 2.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
		else if (visuale % 3 == 2)
			gluLookAt(0.f, 3.3f, 0.5f, 0.f, 0.f, -0.3f, 0.f, 1.f, 0.f);
		glRotatef(0.f, 0.f, 1.f, 0.f);
		glScalef(tmp * 3, tmp * 3, tmp * 3);

		glTranslatef(zurg.X, zurg.height, zurg.Z);
		recursive_render(scene, scene->mRootNode->mChildren[zurg.ID], 1.0); //stampo gli oggetti con gli ID corrispondenti
		glTranslatef(-zurg.X, -zurg.height, -zurg.Z);
		glPopMatrix();
		if (firstDisplay)  // la stopdisplay va chiamata 1 sola volta per display 
		{
			glutTimerFunc(1500 , stopDisplayBoss, numeroPartite);
			firstDisplay = 0;
		}
	}

	if (checkLaserCollision() && !laser.collisione)
	{
		laser.collisione = 1;
		gameManager(4);
		laser.ID = -1;
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (visuale % 3 == 0)
		gluLookAt(0.f, -0.5, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	else if (visuale % 3 == 1)
		gluLookAt(0.f, 2.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	else if (visuale % 3 == 2)
		gluLookAt(0.f, 3.3f, 0.5f, 0.f, 0.f, -0.3f, 0.f, 1.f, 0.f);
	glRotatef(0.f, 0.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(laser.X, laser.height, laser.Z);
	if (laser.ID != -1)
		recursive_render(scene, scene->mRootNode->mChildren[laser.ID], 1.0);
	glTranslatef(-laser.X, -laser.height, -laser.Z);
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (visuale % 3 == 0)
		gluLookAt(0.f, -0.5, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	else if (visuale % 3 == 1)
		gluLookAt(0.f, 2.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	else if (visuale % 3 == 2)
		gluLookAt(0.f, 3.3f, 0.5f, 0.f, 0.f, -0.3f, 0.f, 1.f, 0.f);
	glRotatef(0.f, 0.f, 0.f, 0.f);
	glScalef(tmp / 2, tmp / 2, tmp / 2);
	glTranslatef(astronave.X, astronave.height, astronave.Z);
	recursive_render(scene, scene->mRootNode->mChildren[5], 1.0);
	glTranslatef(-astronave.X, -astronave.height, -astronave.Z);
	glPopMatrix();
	// fine stampa astronave

	// stampa HP pausa punti e power up 
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, Width, 0.0, Height, 0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	strcpy(str, powerUp.c_str());
	glColor3f(1, 0, 0);
	if (powerUpAttivo)
		glColor3f(0, 1, 0);
	displayRasterText2D(50, 30, str);

	strcpy(str, pausa.c_str());
	glColor3f(1, 1, 0);
	displayRasterText2D(50, Height-70, str);

	vita = "HP = " + std::to_string(HP);
	strcpy(str, vita.c_str());
	if (50 < HP)
		glColor3f(0, 1, 0);
	else if (25 < HP && HP <= 50)
		glColor3f(1, 1, 0);
	else
		glColor3f(1, 0, 0);
	displayRasterText2D(Width-150, 30, str);
	glutPostRedisplay();

	punteggio = std::to_string(Punteggio) + " punti";
	strcpy(str, punteggio.c_str());
	glColor3f(1, 1, 0);
	displayRasterText2D(Width/2-30, 30, str);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	do_motion();
	if (abilitaGenerazioneOggetto == 1 && abilitaGenerazioneBoss==0 && primaChiamataBoss) {
		glutTimerFunc(1000, abilitaBoss, 1);
		primaChiamataBoss = 0;
	}
	else if(abilitaGenerazioneOggetto==0 && abilitaGenerazioneBoss==1 && primaChiamataOggetto)
	{
		glutTimerFunc(1000, abilitaOggetto, 1);
		primaChiamataOggetto = 0;
	}
}

void display(void) 
{
	glClear(GL_COLOR_BUFFER_BIT);

	switch (statoGioco)
	{
	case 1:
		reshape(Width,Height);
		drawSchermataIniziale();
		break;
	case 2:
		glEnable(GL_DEPTH_TEST);
		reshape(Width,Height);
		drawGame();
		glDisable(GL_DEPTH_TEST);
		break;
	case 3:
		reshape(Width,Height);
		drawIntro();
		break;
	case 4:
		reshape(Width,Height);
		gameOver();
		break;
	case 5:
		reshape(Width,Height);
		drawRecord();
		break;
	case 6:
		reshape(Width,Height);
		drawSettings();
		break;
	case 7:
		reshape(Width,Height);
		drawPause();
		break;
	case 8:
		reshape(Width,Height);
		drawComandi();
		break;
	case 9:
		reshape(Width,Height);
		drawComandiPause();
		break;
	default:
		break;
	}
	
	glFlush();
	glLoadIdentity();
	glutSwapBuffers();
}

int loadasset(const char* path)
{
	// we are taking one of the postprocessing presets to avoid
	// writing 20 single postprocessing flags here.
	scene = aiImportFile(path, aiProcessPreset_TargetRealtime_Quality);

	if (scene) {
		get_bounding_box(&scene_min, &scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
		return 0;
	}
	return 1;
}

int LoadGLTextures(const aiScene* scene)
{
	ILboolean success;

	/* Before calling ilInit() version should be checked. */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		ILint test = ilGetInteger(IL_VERSION_NUM);
		/// wrong DevIL version ///
		std::string err_msg = "Wrong DevIL version. Old devil.dll in system32/SysWow64?";
		char* cErr_msg = (char*)err_msg.c_str();

		return -1;
	}

	ilInit(); /* Initialization of DevIL */

	//if (scene->HasTextures()) abortGLInit("Support for meshes with embedded textures is not implemented");

	/* getTexture Filenames and Numb of Textures */
	for (unsigned int m = 0; m < scene->mNumMaterials; m++)
	{
		int texIndex = 0;
		aiReturn texFound = AI_SUCCESS;

		aiString path;	// filename

		while (texFound == AI_SUCCESS)
		{
			texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
			textureIdMap[path.data] = NULL; //fill map with textures, pointers still NULL yet
			texIndex++;
		}
	}

	int numTextures = textureIdMap.size();

	/* array with DevIL image IDs */
	ILuint* imageIds = NULL;
	imageIds = new ILuint[numTextures];

	/* generate DevIL Image IDs */
	ilGenImages(numTextures, imageIds); /* Generation of numTextures image names */

	/* create and fill array with GL texture ids */
	textureIds = new GLuint[numTextures];
	glGenTextures(numTextures, textureIds); /* Texture name generation */

	/* define texture path */
	//std::string texturepath = "../../../test/models/Obj/";

	/* get iterator */
	std::map<std::string, GLuint*>::iterator itr = textureIdMap.begin();

	for (int i = 0; i < numTextures; i++)
	{

		//save IL image ID
		std::string filename = (*itr).first;  // get filename
		(*itr).second = &textureIds[i];	  // save texture id for filename in map
		itr++;								  // next texture


		ilBindImage(imageIds[i]); /* Binding of DevIL image name */
		std::string fileloc = basepath + filename;	/* Loading of image */
		success = ilLoadImage((const wchar_t*)fileloc.c_str());

		fprintf(stdout, "Loading Image: %s\n", fileloc.data());

		if (success) /* If no error occured: */
		{
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE); /* Convert every colour component into
			unsigned byte. If your image contains alpha channel you can replace IL_RGB with IL_RGBA */
			if (!success)
			{
				/* Error occured */
				fprintf(stderr, "Couldn't convert image");
				return -1;
			}
			//glGenTextures(numTextures, &textureIds[i]); /* Texture name generation */
			glBindTexture(GL_TEXTURE_2D, textureIds[i]); /* Binding of texture name */
			//redefine standard texture values
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear
			interpolation for magnification filter */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear
			interpolation for minifying filter */
			glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
				ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
				ilGetData()); /* Texture specification */
		}
		else
		{
			/* Error occured */
			fprintf(stderr, "Couldn't load Image: %s\n", fileloc.data());
		}
	}
	ilDeleteImages(numTextures, imageIds); /* Because we have already copied image data into texture data
	we can release memory used by image. */

	//Cleanup
	delete[] imageIds;
	imageIds = NULL;

	//return success;
	return TRUE;
}

int InitGL()		 // per il menu			 // All Setup For OpenGL goes here
{ 
	if (!LoadGLTextures(scene))
	{
		return FALSE;
	}
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);		 // Enables Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);				// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);		// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);			// The Type Of Depth Test To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculation
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);    // Uses default lighting parameters
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_NORMALIZE);
	
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);
	
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	
	return TRUE;					// Initialization Went OK
}

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	
	statoGioco = 3;

	PlaySound(TEXT("ToyStorySong.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

	struct aiLogStream stream;
	
	glutInitWindowSize(Width, Height);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInit(&argc, argv);
	glutCreateWindow("ZURG ATTACK - a toy story game");
	init(); //non da problemi 
	glutIdleFunc(refresh);

	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseClick);
	glutPassiveMotionFunc(passiveMotionFunc);
	glutDisplayFunc(display);
	
	ChangeVolume(0.7, TRUE);
	effects.Load();

	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
	aiAttachLogStream(&stream);

	// ... exactly the same, but this stream will now write the
	// log file to assimp_log.txt
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE, "assimp_log.txt");
	aiAttachLogStream(&stream);

	//loadasset(argv[1]); // DEBUG
	loadasset("./models/ZurgAttack.obj");

	if (!InitGL())
	{
		printf("error");
		fprintf(stderr, "Initialization failed");
		return FALSE;
	}
	glutGet(GLUT_ELAPSED_TIME);
	
	glutMainLoop();

	// cleanup - calling 'aiReleaseImport' is important, as the library 
	// keeps internal resources until the scene is freed again. Not 
	// doing so can cause severe resource leaking.
	aiReleaseImport(scene);

	// We added a log stream to the library, it's our job to disable it
	// again. This will definitely release the last resources allocated
	// by Assimp.
	aiDetachAllLogStreams();
	return 0;
}

/* --------------------------------------------------------------

	 ZURG ATTACK - un videogame arcade ispirato a Toy Story.

	 Progetto realizzato per il corso di INFORMATICA GRAFICA
	 con l'ausilio delle librerie OpenGL, Assimp e DevIL.

	 Politecnico di Torino - anno accademico 2021/22
	 Docenti:
	 Fabrizio Lamberti
	 Alberto Cannavò

	 Ideato e realizzato dagli studenti:
	 Federico Mafrici, s306156
	 Kevin Mascitti, s302286

--------------------------------------------------------------- */