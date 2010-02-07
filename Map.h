#ifndef _GL
#include<windows.h>
#include <cstring>
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glut.h>
#include "GLMetaseq.h"

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glut32.lib")

static const int  Map_Width =10;
static const int Map_Height =23;
static const int Piece_Width=4;
static const int Piece_Height=4;
static const float block_size=10.0f;

typedef struct{
    int del_num;
    int del_y;//消したブロックの中で、最も小さいy座標のブロックのさらに、ひとつ下。（プログラム的座標）
}DelInfo;

class PieceClass;

class FieldClass{

private:
    bool Field[Map_Width][Map_Height];
    unsigned int FieldColor[Map_Width][Map_Height];
public:
    FieldClass(){
	memset(&Field,'\0',sizeof(Field));
	memset(&FieldColor,'\0',sizeof(FieldColor));
    }
    friend void MovePiece(int KeyDown,FieldClass &f_obj,PieceClass &p_obj);
    friend void RolePiece(int KeyDown,FieldClass &f_obj,PieceClass &p_obj);
    DelInfo deletePiece();
    void ShiftPiece(DelInfo del);
void Render(MQO_MODEL &MyModel);
};

class PieceClass{
private:
    bool Piece[Piece_Width][Piece_Height];
    unsigned int PieceColor[Piece_Width][Piece_Height];
    int pos_x,pos_y;
    bool next_Piece[Piece_Width][Piece_Height];
    unsigned int next_PieceColor[Piece_Width][Piece_Height];
public:
    PieceClass(){
	memset(&Piece,'\0',sizeof(Piece));
	memset(&PieceColor,'\0',sizeof(PieceColor));
	pos_x=pos_y=0;
	memset(&next_Piece,'\0',sizeof(next_Piece));
	memset(&next_PieceColor,'\0',sizeof(next_PieceColor));
    }
    void debug(){
for(int i=0;i<Piece_Height;i++)
for(int j=0;j<Piece_Width;j++)
if(Piece[j][i])printf("%d,%d\n",j,i);

    }
    void CreatePiece();
    friend void MovePiece(int KeyDown,FieldClass &f_obj,PieceClass &p_obj);
    friend void RolePiece(int KeyDown,FieldClass &f_obj,PieceClass &p_obj);
    void SwapPiece();
    int GetLeftLimit();
    int GetRightLimit();
    int GetBottomLimit();
void Render(MQO_MODEL &MyModel);

};
#define _GL
#endif 