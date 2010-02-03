#define DEBUG

#ifdef DEBUG
#pragma comment(lib,"libARd.lib")
#pragma comment(lib,"libARgsubd.lib")
#pragma comment(lib,"libARvideod.lib")
#else
#pragma comment(lib,"libAR.lib")
#pragma comment(lib,"libARgsub.lib")
#pragma comment(lib,"libARvideo.lib")
#endif



#include<stdio.h>
#include "Map.h"
#include<AR/ar.h>
#include<AR/param.h>
#include<AR/gsub.h>
#include<AR/video.h>

char *vconf_name="Data/WDM_camera_flipV.xml";
char *pattern_name="Data/patt.shinobi";
char *cparam_name="Data/camera_para.dat";
char *model_name="Data\\Model\\box.mqo";
char *wall_name="Data/Model/surroundblock.mqo";
int patt_id;
double patt_trans[3][4];
double patt_center[2]	={0.0, 0.0};
double patt_width	=80.0;
int thresh		=100.0;
bool isFirst=true;
//model
MQO_MODEL MyModel;
MQO_MODEL WallModel;

LARGE_INTEGER nFreq,nBefore,nAfter;
DWORD dwTime;

void MainLoop();
void KeyEvent(unsigned char key,int x,int y);
void MouseEvent(int button,int state,int x,int y);
void CleanUp();
void DrawObject();
void SetLight();
void SetMaterial();

FieldClass f;
PieceClass p;
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

    if( (patt_id=arLoadPatt(pattern_name)) < 0){
	puts("パターン読み込みエラー");
	return -1;
    }

    argInit(&cparam, 1.0, 0, 0, 0, 0); 
    mqoInit();
    MyModel=mqoCreateModel(model_name,0.05);
    if(MyModel==NULL){
	puts("モデル読み込みエラー");
	return -1;
    }
    WallModel=mqoCreateModel(wall_name,0.05);
    if(WallModel==NULL){
	puts("WallModel Reading Error");
	return -1;
    }

    arVideoCapStart();

    p.CreatePiece();

    p.SwapPiece();

    p.debug();
    arUtilTimerReset();
    argMainLoop(MouseEvent,KeyEvent,MainLoop);

    return 0;


}

void MainLoop()
{
QueryPerformanceFrequency(&nFreq);
QueryPerformanceCounter(&nBefore);
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
	if(isFirst){
	    arGetTransMat(&marker_info[k],patt_center,patt_width,patt_trans);
	}
	else{
	    arGetTransMatCont(&marker_info[k],patt_trans,patt_center,patt_width,patt_trans);
	}
	isFirst=false;
	if(arUtilTimer()>1.0){
	    MovePiece(3,f,p);
	    arUtilTimerReset();
	}
	 f.ShiftPiece(f.deletePiece());

	DrawObject();
    }

    argSwapBuffers();
QueryPerformanceCounter(&nAfter);
dwTime = (DWORD)((nAfter.QuadPart - nBefore.QuadPart) * 1000 / nFreq.QuadPart);
printf("%d fps\n", 1000/dwTime);
}

void WallRender(){
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





}

void DrawObject()
{
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
    WallRender();
    p.Render(MyModel);
    f.Render(MyModel);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

void SetLight(){
    GLfloat light_diffuse[] ={0.5,0.5,0.5,0.2};
    GLfloat light_specular[]={1.0, 1.0, 1.0, 1.0};
    GLfloat light_ambient[] ={0.3, 0.3, 0.3 ,0.1};
    GLfloat light_position[]={100.0, 200.0, 200.0, 0.0};

    glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);
    glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
    glLightfv(GL_LIGHT0,GL_POSITION,light_position);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT0);

}

void SetMaterial(){
    GLfloat mat_diffuse[]   ={1.0, 1.0, 1.0, 1.0};
    GLfloat mat_specular[]  ={0.0, 0.0, 1.0, 1.0};
    GLfloat mat_ambient[]   ={0.0, 0.0, 1.0, 1.0};
    GLfloat shine[]	    ={50.0};

    glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
    glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
    glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
    glMaterialfv(GL_FRONT,GL_SHININESS,shine);



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
    mqoDeleteModel(MyModel);
    mqoDeleteModel(WallModel);
    mqoCleanup();
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}