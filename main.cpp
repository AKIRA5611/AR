#ifdef _WIN32
#define qDEBUG

#ifdef DEBUG
#pragma comment(lib,"libARd.lib")
#pragma comment(lib,"libARgsubd.lib")
#pragma comment(lib,"libARvideod.lib")
#else
#pragma comment(lib,"libAR.lib")
#pragma comment(lib,"libARgsub.lib")
#pragma comment(lib,"libARvideo.lib")
#endif

#pragma comment(lib,"ftgl.lib")
#pragma comment(lib,"freetype2311.lib")
#pragma comment(lib,"libiconv.lib")

#else
#include <time.h>
#include <sys/time.h>

#endif

#include<cstdio>
#include"Map.h"
#include<AR/ar.h>
#include<AR/param.h>
#include<AR/gsub.h>
#include<AR/video.h>
#include"nyar_NyARTransMat.h"
#include<FTGL/FTGLPolygonFont.h>
#include <iconv.h>

//config file or model data name
const char* FONT="Data/PLANA___.TTF";

#ifdef _WIN32
char *vconf_name="Data/WDM_camera_flipV.xml";
#else
char *vconf_name="dev=/dev/video1";
#endif
char *pattern_name="Data/patt.mine";
char *cparam_name="Data/camera_para.dat";
char *wall_name="Data/Model/wall.mqo";
//patt config
int   patt_id;
double patt_trans[3][4];
double patt_center[2]	={0.0, 0.0};
double patt_width	=80.0;
int thresh		=100.0;
bool isFirst=true;
//model
MQO_MODEL Box[box_variey_num];
MQO_MODEL WallModel;
FTFont *font;

#ifndef _WIN32
typedef unsigned long DWORD;
#endif
//extra parameter
DWORD dwTime;
static const DWORD FPS=30;
DWORD fps;
bool GameOver=false;
int score;
const float FONT_SIZE=10.0;
// prototype method

void MainLoop();
void KeyEvent(unsigned char key,int x,int y);
void MouseEvent(int button,int state,int x,int y);
int  Data_Load();
void InitGame();
void SetLight();
void SetMaterial();
void DrawObject();
void CleanUp();
int convert(char *inbuf, char *outbuf, size_t os);
void drawString(FTFont *font, char *str);
//object declaration
FieldClass f;
PieceClass p;
nyar_NyARTransMat_O2_t *nyobj;

#ifndef _WIN32
double gettimeofday_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void Wait(double wait_time){
    double start_time=gettimeofday_sec();
    while(gettimeofday_sec()<wait_time+start_time)
	if(wait_time>0)sleep(1);
}
//overload!!

void FPSCount(DWORD* fps){

    static double before=gettimeofday_sec();
    dobule now=gettimeofday_sec();
    static DWORD fps_ctr=0;

    if(now-before>=1.000){
	before=now;
	*fps=fps_ctr;
	fps_ctr=0;

    }
    fps_ctr++;
}

#else
void Wait(DWORD wait_time){
    DWORD start_time=timeGetTime();
    while(timeGetTime()<wait_time +start_time)
	if(wait_time>0)Sleep(1);

}

void FPSCount(DWORD * fps){

    static DWORD before=timeGetTime();
    DWORD now=timeGetTime();
    static DWORD fps_ctr=0;

    if(now-before>=1000){
	before=now;
	*fps=fps_ctr;
	fps_ctr=0;
    }
    fps_ctr++;
}

#endif

int main(int argc,char**argv){

    ARParam cparam;
    ARParam wparam;
    int xsize,ysize;
    glutInit(&argc,argv);

    if(arVideoOpen(vconf_name)<0){
	puts("ビデオデバイスエラー");
	return -1;
    }

    if(arVideoInqSize(&xsize,&ysize) < 0)return -1;

    if(arParamLoad(cparam_name,1,&wparam)< 0){
	puts("パラメータ読み込み失敗");
	return -1;
    }

    arParamChangeSize(&wparam,xsize,ysize,&cparam);
    arInitCparam(&cparam);
    nyobj = nyar_NyARTransMat_O2_create(&cparam);

    if( (patt_id=arLoadPatt(pattern_name)) < 0){
	puts("パターン読み込みエラー");
	return -1;
    }

    argInit(&cparam, 1.0, 0, 0, 0, 0); 
    mqoInit();

    if(Data_Load()==-1)
	return -1;

    arVideoCapStart();
    InitGame();

    arUtilTimerReset();
#ifdef _WIN32
    TIMECAPS Caps;
    timeGetDevCaps(&Caps, sizeof(TIMECAPS)); // 性能取得
    timeBeginPeriod(Caps.wPeriodMin);
#endif
    argMainLoop(MouseEvent,KeyEvent,MainLoop);
#ifdef _WIN32
    timeEndPeriod(Caps.wPeriodMin);
#endif
    nyar_NyARTransMat_O2_free(nyobj);
    return 0;


}
int  Data_Load(){
    char **strbox=new char* [4];
    for(int i=0;i<box_variey_num;i++)
	strbox[i]=new char [32];

    strcpy_s(strbox[0],32,"Data/Model/box.mqo");
    strcpy_s(strbox[1],32,"Data/Model/bluebox.mqo");
    strcpy_s(strbox[2],32,"Data/Model/redbox.mqo");
    strcpy_s(strbox[3],32,"Data/Model/purplebox.mqo");

    for(int i=0;i<box_variey_num;i++){
	Box[i]=mqoCreateModel(strbox[i],0.05);
	if(Box[i]==NULL){
	    printf("box=%s is not found\n",strbox[i]);
	    return -1;
	}
    }
    WallModel=mqoCreateModel(wall_name,0.5);
    if(WallModel==NULL){
	puts("WallModel is not found");
	return -1;
    }

    font =new FTGLPolygonFont(FONT);
    if(font->Error())exit(1);
    if(!font->FaceSize(FONT_SIZE))exit(1);
    if(!font->CharMap(ft_encoding_unicode))exit(1);


    for(int i=0;i<box_variey_num;i++)
	delete [] strbox[i];
    delete [] strbox;

    return 0;
}


void InitGame(){
    f.Init();
    p.CreatePiece();
    p.SwapPiece();
    score =0;
    GameOver=false;
    isFirst=true;
}

void MainLoop()
{
    //QueryPerformanceFrequency(&nFreq);
    //QueryPerformanceCounter(&nBefore);
    
    DWORD StartTime,EndTime,PassTime;
    double l_StartTime,l_EndTime,l_PassTime;
#ifdef _WIN32
    StartTime=timeGetTime();
#else
    l_StartTime=gettimeofday_sec();
#endif
    ARUint8		*image;
    ARMarkerInfo	*marker_info;
    int			 marker_num;
    int			 j,k;

    if( (image = (ARUint8*)arVideoGetImage() )==NULL){
	arUtilSleep(2);
	return;
    }

    argDrawMode2D();
    argDispImage(image, 0, 0);


    if(arDetectMarker(image, thresh, &marker_info, &marker_num) < 0){
	CleanUp();
	exit(0);
    }
    arVideoCapNext();

    k=-1;
    for(j=0;j<marker_num;j++){   
	if(patt_id==marker_info[j].id){
	    k = (k==-1)  ?   j : k;
	    k = (marker_info[k].cf < marker_info[j].cf)   ?  j: k;
	}
    }

    if(k!=-1) {
	if(isFirst==true)
	    nyar_NyARTransMat_O2_transMat(nyobj,&marker_info[k],patt_center,patt_width,patt_trans);
	else
	    nyar_NyARTransMat_O2_transMatCont(nyobj,&marker_info[k],patt_trans,patt_center,patt_width,patt_trans);

	isFirst=false;

	if(GameOver==false){
	    if(arUtilTimer()>1.0){
		MovePiece(3,f,p); 
		score+=f.ShiftPiece(f.deletePiece());
		arUtilTimerReset();
		GameOver=GameOverCheck(f,p);
	    }
	}
	else{
	    if(arUtilTimer()>15.0)
		InitGame();
	}
	DrawObject();
    }

    argSwapBuffers();
#ifdef _WIN32
    EndTime=timeGetTime();
    PassTime=EndTime-StartTime;
    (1000/FPS>PassTime)?Wait(1000/FPS-PassTime):Wait(0);
    FPSCount(&fps);
    printf("FPS=%d\n",fps);
#else
l_EndTime=gettimeofday_sec();
l_PassTime=l_EndTime-l_StartTime;
    ((double)(1000/FPS)>l_PassTime)?Wait((double)1000/FPS-l_PassTime):Wait(0);
    FPSCount(&fps);
    printf("FPS=%d\n",fps);
#endif
   
}

void WallRender(){
    /*
    for(int i=0;i<Map_Height;i++){
    glPushMatrix();
    glTranslatef(-block_size,-block_size*i,50.0f);
    mqoCallModel(WallModel);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(block_size*Map_Width,-block_size*i,50.0f);
    mqoCallModel(WallModel);
    glPopMatrix();
    }
    for(int i=0;i<Map_Width+2;i++){
    glPushMatrix();
    glTranslatef(block_size*(i-1),-block_size*Map_Height,50.0f);
    mqoCallModel(WallModel);
    glPopMatrix();

    }
    */
    glPushMatrix();
    glTranslatef(-block_size,-9*block_size,Map_depth);
    mqoCallModel(WallModel);
    glPopMatrix();
}

void FontRender(char *str,float pos_x,float pos_y){
    int width=0;
    int i=0;
    while(str[i++]){
	glPushMatrix();
	glTranslatef(110.0f+pos_x,50.0f+pos_y,Map_depth);
	glScalef(3.0f,3.0f,3.0f);
	drawString(font,str);
	//glutStrokeCharacter(GLUT_STROKE_ROMAN,str[i-1]);
	//width+=glutStrokeWidth(GLUT_STROKE_ROMAN,str[i-1]);
	//glutBitmapCharacter(GLUT_BITMAP_9_BY_15,str[i-1]);
	//width+=glutBitmapWidth(GLUT_BITMAP_9_BY_15,str[i-1]);
	glPopMatrix();
    }
}
void DrawObject()
{
    char *str1="Score ";
    char *str2="GameOver";
    char buf[16];
    sprintf(buf,"%d",score);
    double gl_para[16];
    argDrawMode3D();
    argDraw3dCamera(0, 0);

    argConvGlpara(patt_trans,gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(gl_para);
    SetLight();

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glTranslatef(-20.0f,0.0f,20.0f);
    //  glRotatef(0.0,1.0,0.0,0.0);
    if(GameOver==false){
	FontRender(str1,0,0);
        FontRender(buf,60,0);
	WallRender();
	p.Render(Box);
	f.Render(Box);
    }
    else{
	FontRender(str2,0,0);
    }
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

void SetLight(){
    GLfloat light_diffuse[] ={0.2,0.2,0.5,0.2};
    GLfloat light_specular[]={1.0, 1.0, 1.0, 1.0};
    GLfloat light_ambient[] ={0.2, 0.3, 0.3 ,0.1};
    GLfloat light_position[]={100.0, 200.0, 200.0, 0.0};

    glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);
    glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT0,GL_POSITION,light_position);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT0);

}

void SetMaterial(){
    GLfloat mat_diffuse[]   ={1.0, 0.0, 0.0, 0.0};
    GLfloat mat_specular[]  ={0.0, 0.0, 1.0, 1.0};
    GLfloat mat_ambient[]   ={0.0, 0.0, 1.0, 1.0};
    GLfloat shine[]	    ={50.0};

    glMaterialfv(GL_BACK,GL_DIFFUSE,mat_diffuse);
    glMaterialfv(GL_BACK,GL_SPECULAR,mat_specular);
    glMaterialfv(GL_BACK,GL_AMBIENT,mat_ambient);
    glMaterialfv(GL_BACK,GL_SHININESS,shine);


}
void KeyEvent(unsigned char key,int x,int y){
    if(key==0x1b)
	CleanUp(),exit(0);
    if(key=='a')
	MovePiece(2,f,p);
    if(key=='d')
	MovePiece(4,f,p);
    if(key==' ')
	RolePiece(0,f,p);
    if(key=='s')
	MovePiece(3,f,p);
}

void MouseEvent(int button,int state,int x,int y){
    //printf("ボタン:%d 状態：%d 座標：(x、y)=%d,%d\n",button,state,x,y);

}

void CleanUp(){
    nyar_NyARTransMat_O2_free(nyobj);
    mqoDeleteModel(WallModel);
    for (int i=0;i<box_variey_num;i++){
	mqoDeleteModel(Box[i]);
    }
    mqoCleanup();
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

int convert(char *inbuf, char *outbuf, size_t os)
{
    iconv_t conv;
    char    *in, *out;
    size_t  is;

    in = inbuf;
    out = outbuf;
    is = (size_t)(strlen(inbuf)+1);

    if ((conv = iconv_open("WCHAR_T", "EUC-JP")) == (iconv_t)(-1)) return (1);	// can't open 
    if (iconv(conv, &in, &is, &out, &os) == (size_t)(-1)) return (2);	// can't convert
    iconv_close(conv);
    return (0);
}

void drawString(FTFont *font, char *str)
{
    char   outstr[256];
    float  llx, lly, llz;
    float  urx, ury, urz;

    if (convert(str, outstr, 256)) return;

    font->BBox((wchar_t *)outstr, llx, lly, llz, urx, ury, urz);
    glPushMatrix();
    glTranslatef(-(llx+urx)/2.0, -(lly+ury)/2.0, 0.0);
    font->Render((wchar_t *)outstr);
    glPopMatrix();
}