/*
�S�̓I�ȕ\��

�X�R�A

�Q�[���Z���N�g

NPC

�ΐ�

��̈��

�Q�[���I�[�o�[

�g��

���_�ړ�

�����߃Q�[��

�悯�Q�[��
*/
#include <cstdlib>
#include "Map.h"

void PieceClass::Render(MQO_MODEL MyModel[]){

    for (int i=0;i<Piece_Height;i++)
	for (int j=0;j<Piece_Width;j++)
	    if(Piece[j][i]==true){
		glPushMatrix();
		glTranslatef(block_size*(j+pos_x),-block_size*(i+pos_y),Map_depth);
		mqoCallModel(MyModel[PieceColor]);
		glPopMatrix();
	    }

}

void FieldClass::Render(MQO_MODEL MyModel[]){

    for (int i=0;i<Map_Height;i++)
	for (int j=0;j<Map_Width;j++)
	    if(Field[j][i]==true){
		glPushMatrix();
		glTranslatef(block_size*j,-block_size*i,Map_depth);
		mqoCallModel( MyModel[FieldColor[j][i]] );
		glPopMatrix();
	    }



}
void PieceClass::CreatePiece(){

    switch(rand()%7){
	//���P�[�X����̂��@�s�� 14
	/*
	Color
	0 green
	1 blue
	2 red
	3 purple
	*/
case 1:
    next_Piece[1][1]=next_Piece[2][1]=next_Piece[1][2]=next_Piece[2][2]=true;
    break;
case 2:
    next_Piece[1][0]=next_Piece[1][1]=next_Piece[1][2]=next_Piece[1][3]=true;
    break;
case 3:
    next_Piece[1][0]=next_Piece[1][1]=next_Piece[1][2]=next_Piece[0][2]=true;
    break;
case 4:
    next_Piece[1][0]=next_Piece[1][1]=next_Piece[1][2]=next_Piece[2][2]=true;
    break;
case 5:
    next_Piece[1][1]=next_Piece[0][1]=next_Piece[1][2]=next_Piece[2][2]=true;
    break;
case 6:
    next_Piece[1][1]=next_Piece[2][1]=next_Piece[0][2]=next_Piece[1][2]=true;
    break;
case 0:
    next_Piece[1][1]=next_Piece[2][1]=next_Piece[0][1]=next_Piece[1][0]=true;
    }

    next_PieceColor=rand()%4;

}

bool GameOverCheck(FieldClass &f,PieceClass &p){

    for(int i=0;i<Piece_Width;i++)
	for (int j=0;j<Piece_Height;j++)
	    if(p.Piece[i][j])
		if(f.Field[p.pos_x+i][p.pos_y+j]==true)
		    return true;

return false;
}

void MovePiece(int KeyDown,FieldClass &f_obj,PieceClass &p_obj){
    //Stop and write Filed method is not written yet......
    int x_limit;
    int y_limit;
    int count=0;
    int check_count=0;
    switch(KeyDown){
case 2:
    x_limit=p_obj.GetLeftLimit();
    if(p_obj.pos_x +x_limit -1 >= 0)
	for(int i=0;i<Piece_Height;i++){
	    if(p_obj.Piece[x_limit][i]){
		count++;
		if(f_obj.Field[p_obj.pos_x + x_limit - 1][p_obj.pos_y+i]==false)
		    check_count++;
	    }

	    if(p_obj.Piece[x_limit+1][i])
		if(f_obj.Field[x_limit+p_obj.pos_x][i+p_obj.pos_y]==true)
		    count=0;//���̏����́@�������Ȃ�
	}
	if(check_count==count && check_count!=0 && count!=0)
	    p_obj.pos_x--;
	break;
case 3:
    y_limit =p_obj.GetBottomLimit();
    if(p_obj.pos_y + y_limit + 1 <Map_Height)
	for(int i=0;i<Piece_Width;i++){
	    if(p_obj.Piece[i][y_limit]){
		count++;
		if(f_obj.Field[i + p_obj.pos_x][y_limit +p_obj.pos_y +1]==false)
		    check_count++;
	    }
	    if(p_obj.Piece[i][y_limit-1])
		if(f_obj.Field[i+p_obj.pos_x][y_limit+p_obj.pos_y]==true){
		    count=0;//���̏����́@�������Ȃ�
		    break;
		}
	}

	if(check_count==count && check_count!=0 && count!=0)
	    p_obj.pos_y++;
	else{

	    for(int i=0;i<Piece_Width;i++)
		for(int j=0;j<Piece_Height;j++)
		    if(p_obj.Piece[i][j]){
			f_obj.Field[i+p_obj.pos_x][j+p_obj.pos_y]=p_obj.Piece[i][j];
			f_obj.FieldColor[i+p_obj.pos_x][j+p_obj.pos_y]=p_obj.PieceColor;
		    }
		    p_obj.CreatePiece();
		    p_obj.SwapPiece();

	}
	//p_obj.pos_x++;
	break;
case 4:
    x_limit=p_obj.GetRightLimit();
    if(p_obj.pos_x + x_limit +1 < Map_Width)
	for(int i=0;i<Piece_Height;i++){
	    if(p_obj.Piece[x_limit][i]){
		count++;
		if(f_obj.Field[p_obj.pos_x + x_limit + 1][p_obj.pos_y+i]==false)
		    check_count++;
	    }
	    if(p_obj.Piece[x_limit-1][i])
		if(f_obj.Field[x_limit+p_obj.pos_x][i+p_obj.pos_y]==true){
		    count=0;//���̏����́@�������Ȃ�
		    break;
		}
	}
	if(check_count==count  && check_count!=0 && count!=0)
	    p_obj.pos_x++;
	break;
    }

}

int PieceClass::GetLeftLimit(){
    int position=0;//x
    for(int i=Piece_Width-1;i>=0;i--)
	for (int j=0;j<Piece_Height;j++)
	    if(Piece[i][j])position =i;

    return position;
}
//�����悤�ȏ����𓯂��悤�ɏ����Ȃ��̂͂ނ���

int PieceClass::GetRightLimit(){
    int position=0;//x
    for(int i=0;i<Piece_Width;i++)
	for (int j=0;j<Piece_Height;j++)
	    if(Piece[i][j])position =i;

    return position;
}




int PieceClass::GetBottomLimit(){
    int position=0;//x
    for(int i=0;i<Piece_Height;i++)
	for (int j=0;j<Piece_Width;j++)
	    if(Piece[j][i])position =i;

    return position;
}


void PieceClass::SwapPiece(){

    for(int i=0;i<Piece_Width;i++)
	for(int j=0;j<Piece_Height;j++){
	    Piece[i][j]=next_Piece[i][j];
	    next_Piece[i][j]=false;
	}

	pos_x=pos_y=0;
	PieceColor=next_PieceColor;

}
DelInfo FieldClass::deletePiece(){
    int del_num=0,del_y=0;
    for(int j=Map_Height-1;j>=0;j--){
	int count=0;
	for(int i=0;i<Map_Width;i++)
	    if(Field[i][j])count++;

	if(count==0)break;
	else if(Map_Width==count){
	    for(int i=0;i<Map_Width;i++)
		Field[i][j]=false;
	    del_num++;
	    del_y=j;
	}
    }
    DelInfo Del;
    Del.del_num=del_num;
    Del.del_y=del_y-1;

    return Del;
}
int FieldClass::ShiftPiece(DelInfo del){
    int shift_num=del.del_num;
    int count=0;
    for(int j=del.del_y;j>=0;j--){
	for (int i=0;i<Map_Width;i++)
	    if(Field[i][j])count++;

	if(count==0)break;

	for(int i=0;i<Map_Width;i++){
	    Field[i][j+del.del_num]=Field[i][j],Field[i][j]=false;
	    FieldColor[i][j+del.del_num]=FieldColor[i][j];
	}
    }

return del.del_num*10;
}
/* 5 block ver
void RolePiece(int KeyDown,FieldClass &f_obj,PieceClass &p_obj){
//����[2][2]�̎����90�x��]�����A���W�̌��_�����[�Ɉڂ��ƁA�����Ȃ�܂��B
//If upper Limit bug happen ,I write code again!
PieceClass tmp=p_obj;
for (int i=0;i<Piece_Width;i++)
for(int j=0;j<Piece_Height;j++)
if(tmp.Piece[i][j]){
if(p_obj.pos_x+j<0 || p_obj.pos_x+j>=Map_Width
|| p_obj.pos_y + 4 -i>=Map_Height)
return;
if(f_obj.Field[p_obj.pos_x+j][p_obj.pos_y + 4 -i])
return;
}


for (int i=0;i<Piece_Width;i++)
for(int j=0;j<Piece_Height;j++){
if(p_obj.Piece[i][j]){
tmp.Piece[i][j]=false;
tmp.Piece[j][4-i]=true;
}
p_obj=tmp;
}	
}
*/

void RolePiece(int KeyDown,FieldClass &f_obj,PieceClass &p_obj){
    //����[2][2]�̎����90�x��]�����A���W�̌��_�����[�Ɉڂ��ƁA�����Ȃ�܂��B
    //If upper Limit bug happen ,I write code again!


    for (int i=0;i<Piece_Width;i++)
	for(int j=0;j<Piece_Height;j++)
	    if(p_obj.Piece[i][j]){
		if(p_obj.pos_x+3-j<0 || p_obj.pos_x+3-j>=Map_Width
		    || p_obj.pos_y + i>=Map_Height)
		    return;
		if(f_obj.Field[p_obj.pos_x+3-j][p_obj.pos_y + i]==true)
		    return;
	    }

	    PieceClass tmp;
	    tmp.PieceColor=p_obj.PieceColor;
	    tmp.pos_x=p_obj.pos_x;
	    tmp.pos_y=p_obj.pos_y;

	    for (int i=0;i<Piece_Width;i++)
		for(int j=0;j<Piece_Height;j++){
		    if(p_obj.Piece[i][j]){
			tmp.Piece[3-j][i]=true;
		    }
		}	

		p_obj=tmp;
}

void FieldClass::Init(){
memset(Field,'\0',sizeof(Field));
memset(FieldColor,'\0',sizeof(FieldColor));
}