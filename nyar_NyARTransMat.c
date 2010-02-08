/* 
 * PROJECT: NyARToolkit - C version NyARTransMat function.
 * --------------------------------------------------------------------------------
 * This work is based on the original ARToolKit developed by
 *   Hirokazu Kato
 *   Mark Billinghurst
 *   HITLab, University of Washington, Seattle
 * http://www.hitl.washington.edu/artoolkit/
 *
 * The NyARTransMat is sub project of NyARToolkit.
 * This library includes high-speed-TransMat function which 3 times faster than original model.
 * Copyright (C)2008 R.Iizuka
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this framework; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * For further information please contact.
 *	http://nyatla.jp/nyatoolkit/
 *	<airmail(at)ebony.plala.or.jp>
 * 
 */
#include <AR/matrix.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <math.h>
#include <assert.h>
#include "nyar_NyARTransMat.h"
#include <string.h>


#define nyar_FALSE (0!=0)
#define nyar_TRUE (0==0)

#define nyar_OnException(L) printf("%s:%d\n",__FILE__,__LINE__);goto L;

typedef struct nyar_NyARTransRot_O3 nyar_NyARTransRot_O3_t;
/*********************************************************************************

nyar_NyARTransRot_O3擬似クラス

*********************************************************************************/
struct nyar_NyARTransRot_O3{
	int number_of_vertex;
	double array[9];
	const ARParam* cparam;
	struct{
		ARMat* check_dir_NyARMat;
	}wk;
};




nyar_NyARTransRot_O3_t* nyar_NyARTransRot_O3_create(
	const ARParam* i_param,
	int i_number_of_vertex)
{
	nyar_NyARTransRot_O3_t* inst;
	/*4以外の頂点数は処理しない*/
	assert(i_number_of_vertex==4);

	inst=(nyar_NyARTransRot_O3_t*)malloc(sizeof(nyar_NyARTransRot_O3_t));
	inst->number_of_vertex=i_number_of_vertex;
	inst->cparam=i_param;
	/*ワークオブジェクト*/
	inst->wk.check_dir_NyARMat=arMatrixAlloc( 3, 3 );
	return inst;
}

void nyar_NyARTransRot_O3_free(
							   nyar_NyARTransRot_O3_t* i_inst)
{
	arMatrixFree(i_inst->wk.check_dir_NyARMat);
	free(i_inst);
}

static int check_dir(
	 nyar_NyARTransRot_O3_t* i_inst,
	 double dir[],
	 const double st[],
	 const double ed[],
	 const double cpara[])
{
	double    h;
	int       i, j;
	double world[6];
	double camera[4];
	double v;

	ARMat* mat_a = i_inst->wk.check_dir_NyARMat;
	for(j=0;j<3;j++){
		for(i=0;i<3;i++){
			mat_a->m[j*3+i]=cpara[j*4+i];//m[j*3+i] = cpara[j][i];
		}	    
	}
	arMatrixSelfInv(mat_a);
	world[0] = mat_a->m[0*3+0]*st[0]*10.0+ mat_a->m[0*3+1]*st[1]*10.0+ mat_a->m[0*3+2]*10.0;//mat_a->m[0]*st[0]*10.0+ mat_a->m[1]*st[1]*10.0+ mat_a->m[2]*10.0;
	world[1] = mat_a->m[1*3+0]*st[0]*10.0+ mat_a->m[1*3+1]*st[1]*10.0+ mat_a->m[1*3+2]*10.0;//mat_a->m[3]*st[0]*10.0+ mat_a->m[4]*st[1]*10.0+ mat_a->m[5]*10.0;
	world[2] = mat_a->m[2*3+0]*st[0]*10.0+ mat_a->m[2*3+1]*st[1]*10.0+ mat_a->m[2*3+2]*10.0;//mat_a->m[6]*st[0]*10.0+ mat_a->m[7]*st[1]*10.0+ mat_a->m[8]*10.0;
	world[3] = world[0] + dir[0];
	world[4] = world[1] + dir[1];
	world[5] = world[2] + dir[2];
	//</Optimize>

	for( i = 0; i < 2; i++ ) {
		h = cpara[2*4+0] * world[i*3+0]+ cpara[2*4+1] * world[i*3+1]+ cpara[2*4+2] * world[i*3+2];
		if( h == 0.0 ){
			nyar_OnException(EXCEPTION_HANDLER);
		}
		camera[i*2+0] = (cpara[0*4+0] * world[i*3+0]+ cpara[0*4+1] * world[i*3+1]+ cpara[0*4+2] * world[i*3+2]) / h;
		camera[i*2+1] = (cpara[1*4+0] * world[i*3+0]+ cpara[1*4+1] * world[i*3+1]+ cpara[1*4+2] * world[i*3+2]) / h;
	}
	//<Optimize>
	//v[0][0] = ed[0] - st[0];
	//v[0][1] = ed[1] - st[1];
	//v[1][0] = camera[1][0] - camera[0][0];
	//v[1][1] = camera[1][1] - camera[0][1];
	v=(ed[0]-st[0])*(camera[2]-camera[0])+(ed[1]-st[1])*(camera[3]-camera[1]);
	//</Optimize>
	if(v<0) {//if( v[0][0]*v[1][0] + v[0][1]*v[1][1] < 0 ) {
		dir[0] = -dir[0];
		dir[1] = -dir[1];
		dir[2] = -dir[2];
	}
	return nyar_TRUE;
EXCEPTION_HANDLER:
	return nyar_FALSE;
}


/*int check_rotation( double rot[2][3] )*/
static int check_rotation(
	double rot[2][3])
{
	double v1[3],v2[3],v3[3];
	double  ca, cb, k1, k2, k3, k4;
	double  a, b, c, d;
	double  p1, q1, r1;
	double  p2, q2, r2;
	double  p3, q3, r3;
	double  p4, q4, r4;
	double  w;
	double  e1, e2, e3, e4;
	int     f;

	v1[0] = rot[0][0];
	v1[1] = rot[0][1];
	v1[2] = rot[0][2];
	v2[0] = rot[1][0];
	v2[1] = rot[1][1];
	v2[2] = rot[1][2];
	v3[0] = v1[1]*v2[2] - v1[2]*v2[1];
	v3[1] = v1[2]*v2[0] - v1[0]*v2[2];
	v3[2] = v1[0]*v2[1] - v1[1]*v2[0];
	w=sqrt( v3[0]*v3[0]+v3[1]*v3[1]+v3[2]*v3[2] );
	if( w == 0.0 ){
		nyar_OnException(EXCEPTION_HANDLER);
	}
	v3[0] /= w;
	v3[1] /= w;
	v3[2] /= w;

	cb = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
	if( cb < 0 ) cb *= -1.0;
	ca = (sqrt(cb+1.0) + sqrt(1.0-cb)) * 0.5;

	if( v3[1]*v1[0] - v1[1]*v3[0] != 0.0 ) {
		f = 0;
	}
	else {
		if( v3[2]*v1[0] - v1[2]*v3[0] != 0.0 ) {
			w = v1[1]; v1[1] = v1[2]; v1[2] = w;
			w = v3[1]; v3[1] = v3[2]; v3[2] = w;
			f = 1;
		}
		else {
			w = v1[0]; v1[0] = v1[2]; v1[2] = w;
			w = v3[0]; v3[0] = v3[2]; v3[2] = w;
			f = 2;
		}
	}
	if( v3[1]*v1[0] - v1[1]*v3[0] == 0.0 ){
		nyar_OnException(EXCEPTION_HANDLER);
	}
	k1 = (v1[1]*v3[2] - v3[1]*v1[2]) / (v3[1]*v1[0] - v1[1]*v3[0]);
	k2 = (v3[1] * ca) / (v3[1]*v1[0] - v1[1]*v3[0]);
	k3 = (v1[0]*v3[2] - v3[0]*v1[2]) / (v3[0]*v1[1] - v1[0]*v3[1]);
	k4 = (v3[0] * ca) / (v3[0]*v1[1] - v1[0]*v3[1]);

	a = k1*k1 + k3*k3 + 1;
	b = k1*k2 + k3*k4;
	c = k2*k2 + k4*k4 - 1;

	d = b*b - a*c;
	if( d < 0 ){
		nyar_OnException(EXCEPTION_HANDLER);
	}
	r1 = (-b + sqrt(d))/a;
	p1 = k1*r1 + k2;
	q1 = k3*r1 + k4;
	r2 = (-b - sqrt(d))/a;
	p2 = k1*r2 + k2;
	q2 = k3*r2 + k4;
	if( f == 1 ) {
		w = q1; q1 = r1; r1 = w;
		w = q2; q2 = r2; r2 = w;
		w = v1[1]; v1[1] = v1[2]; v1[2] = w;
		w = v3[1]; v3[1] = v3[2]; v3[2] = w;
		f = 0;
	}
	if( f == 2 ) {
		w = p1; p1 = r1; r1 = w;
		w = p2; p2 = r2; r2 = w;
		w = v1[0]; v1[0] = v1[2]; v1[2] = w;
		w = v3[0]; v3[0] = v3[2]; v3[2] = w;
		f = 0;
	}

	if( v3[1]*v2[0] - v2[1]*v3[0] != 0.0 ) {
		f = 0;
	}else {
		if( v3[2]*v2[0] - v2[2]*v3[0] != 0.0 ) {
			w = v2[1]; v2[1] = v2[2]; v2[2] = w;
			w = v3[1]; v3[1] = v3[2]; v3[2] = w;
			f = 1;
		}
		else {
			w = v2[0]; v2[0] = v2[2]; v2[2] = w;
			w = v3[0]; v3[0] = v3[2]; v3[2] = w;
			f = 2;
		}
	}
	if( v3[1]*v2[0] - v2[1]*v3[0] == 0.0 ){
		nyar_OnException(EXCEPTION_HANDLER);
	}
	k1 = (v2[1]*v3[2] - v3[1]*v2[2]) / (v3[1]*v2[0] - v2[1]*v3[0]);
	k2 = (v3[1] * ca) / (v3[1]*v2[0] - v2[1]*v3[0]);
	k3 = (v2[0]*v3[2] - v3[0]*v2[2]) / (v3[0]*v2[1] - v2[0]*v3[1]);
	k4 = (v3[0] * ca) / (v3[0]*v2[1] - v2[0]*v3[1]);

	a = k1*k1 + k3*k3 + 1;
	b = k1*k2 + k3*k4;
	c = k2*k2 + k4*k4 - 1;

	d = b*b - a*c;
	if( d < 0 ){
		nyar_OnException(EXCEPTION_HANDLER);
	}
	r3 = (-b + sqrt(d))/a;
	p3 = k1*r3 + k2;
	q3 = k3*r3 + k4;
	r4 = (-b - sqrt(d))/a;
	p4 = k1*r4 + k2;
	q4 = k3*r4 + k4;
	if( f == 1 ) {
		w = q3; q3 = r3; r3 = w;
		w = q4; q4 = r4; r4 = w;
		w = v2[1]; v2[1] = v2[2]; v2[2] = w;
		w = v3[1]; v3[1] = v3[2]; v3[2] = w;
		f = 0;
	}
	if( f == 2 ) {
		w = p3; p3 = r3; r3 = w;
		w = p4; p4 = r4; r4 = w;
		w = v2[0]; v2[0] = v2[2]; v2[2] = w;
		w = v3[0]; v3[0] = v3[2]; v3[2] = w;
		f = 0;
	}

	e1 = p1*p3+q1*q3+r1*r3;
	if( e1 < 0 ){
		e1 = -e1;
	}
	e2 = p1*p4+q1*q4+r1*r4;
	if( e2 < 0 ){
		e2 = -e2;
	}
	e3 = p2*p3+q2*q3+r2*r3;
	if( e3 < 0 ){
		e3 = -e3;
	}
	e4 = p2*p4+q2*q4+r2*r4;
	if( e4 < 0 ){
		e4 = -e4;
	}
	if( e1 < e2 ) {
		if( e1 < e3 ) {
			if( e1 < e4 ) {
				rot[0][0] = p1;
				rot[0][1] = q1;
				rot[0][2] = r1;
				rot[1][0] = p3;
				rot[1][1] = q3;
				rot[1][2] = r3;
			}
			else {
				rot[0][0] = p2;
				rot[0][1] = q2;
				rot[0][2] = r2;
				rot[1][0] = p4;
				rot[1][1] = q4;
				rot[1][2] = r4;
			}
		}
		else {
			if( e3 < e4 ) {
				rot[0][0] = p2;
				rot[0][1] = q2;
				rot[0][2] = r2;
				rot[1][0] = p3;
				rot[1][1] = q3;
				rot[1][2] = r3;
			}
			else {
				rot[0][0] = p2;
				rot[0][1] = q2;
				rot[0][2] = r2;
				rot[1][0] = p4;
				rot[1][1] = q4;
				rot[1][2] = r4;
			}
		}
	}
	else {
		if( e2 < e3 ) {
			if( e2 < e4 ) {
				rot[0][0] = p1;
				rot[0][1] = q1;
				rot[0][2] = r1;
				rot[1][0] = p4;
				rot[1][1] = q4;
				rot[1][2] = r4;
			}
			else {
				rot[0][0] = p2;
				rot[0][1] = q2;
				rot[0][2] = r2;
				rot[1][0] = p4;
				rot[1][1] = q4;
				rot[1][2] = r4;
			}
		}
		else {
			if( e3 < e4 ) {
				rot[0][0] = p2;
				rot[0][1] = q2;
				rot[0][2] = r2;
				rot[1][0] = p3;
				rot[1][1] = q3;
				rot[1][2] = r3;
			}
			else {
				rot[0][0] = p2;
				rot[0][1] = q2;
				rot[0][2] = r2;
				rot[1][0] = p4;
				rot[1][1] = q4;
				rot[1][2] = r4;
			}
		}
	}
	return nyar_TRUE;
EXCEPTION_HANDLER:
	return nyar_FALSE;
}  

static void nyar_NyARTransRot_O3_arGetRot(
	const double a,
	const double b,
	const double c,
	double o_rot[9])
{
	double   sina, sinb, sinc;
	double   cosa, cosb, cosc;
	double CACA,SASA,SACA,SASB,CASB;

	sina = sin(a);
	cosa = cos(a);
	sinb = sin(b);
	cosb = cos(b);
	sinc = sin(c);
	cosc = cos(c);
	//Optimize
	CACA=cosa*cosa;
	SASA=sina*sina;
	SACA=sina*cosa;
	SASB=sina*sinb;
	CASB=cosa*sinb;

	o_rot[0] = CACA*cosb*cosc+SASA*cosc+SACA*cosb*sinc-SACA*sinc;
	o_rot[1] = -CACA*cosb*sinc-SASA*sinc+SACA*cosb*cosc-SACA*cosc;
	o_rot[2] = CASB;
	o_rot[3] = SACA*cosb*cosc-SACA*cosc+SASA*cosb*sinc+CACA*sinc;
	o_rot[4] = -SACA*cosb*sinc+SACA*sinc+SASA*cosb*cosc+CACA*cosc;
	o_rot[5] = SASB;
	o_rot[6] = -CASB*cosc-SASB*sinc;
	o_rot[7] = CASB*sinc-SASB*cosc;
	o_rot[8] = cosb;
}

static int nyar_NyARTransRot_O3_arGetAngle(
	nyar_NyARTransRot_O3_t* i_inst,
	double o_abc[3])
{
	double      a, b, c,tmp;
	double      sina, cosa, sinb, cosb, sinc, cosc;
	double* rot=i_inst->array;
	if( rot[8] > 1.0 ) {//<Optimize/>if( rot[2][2] > 1.0 ) {
		rot[8] = 1.0;//<Optimize/>rot[2][2] = 1.0;
	}else if( rot[8] < -1.0 ) {//<Optimize/>}else if( rot[2][2] < -1.0 ) {
		rot[8] = -1.0;//<Optimize/>rot[2][2] = -1.0;
	}
	cosb = rot[8];//<Optimize/>cosb = rot[2][2];
	b = acos( cosb );
	sinb = sin( b );
	if( b >= 0.000001 || b <= -0.000001) {
		cosa = rot[2] / sinb;//<Optimize/>cosa = rot[0][2] / sinb;
		sina = rot[5] / sinb;//<Optimize/>sina = rot[1][2] / sinb;
		if( cosa > 1.0 ) {
			/* printf("cos(alph) = %f\n", cosa); */
			cosa = 1.0;
			sina = 0.0;
		}
		if( cosa < -1.0 ) {
			/* printf("cos(alph) = %f\n", cosa); */
			cosa = -1.0;
			sina =  0.0;
		}
		if( sina > 1.0 ) {
			/* printf("sin(alph) = %f\n", sina); */
			sina = 1.0;
			cosa = 0.0;
		}
		if( sina < -1.0 ) {
			/* printf("sin(alph) = %f\n", sina); */
			sina = -1.0;
			cosa =  0.0;
		}
		a = acos( cosa );
		if( sina < 0 ){
			a = -a;
		}
		//<Optimize>
		//sinc =  (rot[2][1]*rot[0][2]-rot[2][0]*rot[1][2])/ (rot[0][2]*rot[0][2]+rot[1][2]*rot[1][2]);
		//cosc =  -(rot[0][2]*rot[2][0]+rot[1][2]*rot[2][1])/ (rot[0][2]*rot[0][2]+rot[1][2]*rot[1][2]);
		tmp = (rot[2]*rot[2]+rot[5]*rot[5]);
		sinc =  (rot[7]*rot[2]-rot[6]*rot[5])/ tmp;
		cosc =  -(rot[2]*rot[6]+rot[5]*rot[7])/ tmp;
		//</Optimize>

		if( cosc > 1.0 ) {
			/* printf("cos(r) = %f\n", cosc); */
			cosc = 1.0;
			sinc = 0.0;
		}
		if( cosc < -1.0 ) {
			/* printf("cos(r) = %f\n", cosc); */
			cosc = -1.0;
			sinc =  0.0;
		}
		if( sinc > 1.0 ) {
			/* printf("sin(r) = %f\n", sinc); */
			sinc = 1.0;
			cosc = 0.0;
		}
		if( sinc < -1.0 ) {
			/* printf("sin(r) = %f\n", sinc); */
			sinc = -1.0;
			cosc =  0.0;
		}
		c = acos( cosc );
		if( sinc < 0 ){
			c = -c;
		}
	}else {
		a = b = 0.0;
		cosa = cosb = 1.0;
		sina = sinb = 0.0;
		cosc = rot[0];//<Optimize/>cosc = rot[0][0];
		sinc = rot[1];//<Optimize/>sinc = rot[1][0];
		if( cosc > 1.0 ) {
			/* printf("cos(r) = %f\n", cosc); */
			cosc = 1.0;
			sinc = 0.0;
		}
		if( cosc < -1.0 ) {
			/* printf("cos(r) = %f\n", cosc); */
			cosc = -1.0;
			sinc =  0.0;
		}
		if( sinc > 1.0 ) {
			/* printf("sin(r) = %f\n", sinc); */
			sinc = 1.0;
			cosc = 0.0;
		}
		if( sinc < -1.0 ) {
			/* printf("sin(r) = %f\n", sinc); */
			sinc = -1.0;
			cosc =  0.0;
		}
		c = acos( cosc );
		if( sinc < 0 ){
			c = -c;
		}
	}
	o_abc[0]=a;//wa.value=a;//*wa = a;
	o_abc[1]=b;//wb.value=b;//*wb = b;
	o_abc[2]=c;//wc.value=c;//*wc = c;
	return nyar_TRUE;
}    

static int nyar_NyARTransRot_O3_arGetInitRot(
	nyar_NyARTransRot_O3_t* i_inst,
	const ARMarkerInfo* marker_info)
{
	const double* cpara=&(i_inst->cparam->mat[0][0]);
	double  wdir[3][3];
	double  w, w1, w2, w3;
	int     dir;
	int     j;
	double* rot=i_inst->array;

	dir = marker_info->dir;

	for( j = 0; j < 2; j++ ) {
		w1 = marker_info->line[(4-dir+j)%4][0] * marker_info->line[(6-dir+j)%4][1]- marker_info->line[(6-dir+j)%4][0] * marker_info->line[(4-dir+j)%4][1];
		w2 = marker_info->line[(4-dir+j)%4][1] * marker_info->line[(6-dir+j)%4][2]- marker_info->line[(6-dir+j)%4][1] * marker_info->line[(4-dir+j)%4][2];
		w3 = marker_info->line[(4-dir+j)%4][2] * marker_info->line[(6-dir+j)%4][0]- marker_info->line[(6-dir+j)%4][2] * marker_info->line[(4-dir+j)%4][0];

		wdir[j][0] =  w1*(cpara[0*4+1]*cpara[1*4+2]-cpara[0*4+2]*cpara[1*4+1])+  w2*cpara[1*4+1]-  w3*cpara[0*4+1];
		wdir[j][1] = -w1*cpara[0*4+0]*cpara[1*4+2]+  w3*cpara[0*4+0];
		wdir[j][2] =  w1*cpara[0*4+0]*cpara[1*4+1];
		w = sqrt( wdir[j][0]*wdir[j][0]+ wdir[j][1]*wdir[j][1]+ wdir[j][2]*wdir[j][2] );
		wdir[j][0] /= w;
		wdir[j][1] /= w;
		wdir[j][2] /= w;
	}

	//以下3ケースは、計算エラーのときは例外が発生する。
	if(!check_dir(i_inst,wdir[0], marker_info->vertex[(4-dir)%4],marker_info->vertex[(5-dir)%4], cpara)){
		nyar_OnException(EXCEPTION_HANDLER);
	}

	if(!check_dir(i_inst,wdir[1], marker_info->vertex[(7-dir)%4],marker_info->vertex[(4-dir)%4], cpara)){
		nyar_OnException(EXCEPTION_HANDLER);
	}

	if(!check_rotation(wdir)){
		nyar_OnException(EXCEPTION_HANDLER);
	}


	wdir[2][0] = wdir[0][1]*wdir[1][2] - wdir[0][2]*wdir[1][1];
	wdir[2][1] = wdir[0][2]*wdir[1][0] - wdir[0][0]*wdir[1][2];
	wdir[2][2] = wdir[0][0]*wdir[1][1] - wdir[0][1]*wdir[1][0];
	w = sqrt( wdir[2][0]*wdir[2][0]+ wdir[2][1]*wdir[2][1]+ wdir[2][2]*wdir[2][2] );
	wdir[2][0] /= w;
	wdir[2][1] /= w;
	wdir[2][2] /= w;

	rot[0] = wdir[0][0];
	rot[3] = wdir[0][1];
	rot[6] = wdir[0][2];
	rot[1] = wdir[1][0];
	rot[4] = wdir[1][1];
	rot[7] = wdir[1][2];
	rot[2] = wdir[2][0];
	rot[5] = wdir[2][1];
	rot[8] = wdir[2][2];
	return nyar_TRUE;
EXCEPTION_HANDLER:
	return nyar_FALSE;
	//</Optimize>    
}




#define MD_PI         3.14159265358979323846
static double nyar_NyARTransRot_O3_modifyMatrix(
	nyar_NyARTransRot_O3_t* i_inst,
	double trans[],
	double vertex[][3],
	double pos2d[][2])
{
	double    factor;
	double    a2, b2, c2;
	double    ma = 0.0, mb = 0.0, mc = 0.0;
	double    h, x, y;
	double    err, minerr=0;
	int       t1, t2, t3;
	int       s1 = 0, s2 = 0, s3 = 0;
	int i,j;
	double CACA,SASA,SACA,CA,SA;
	double CACACB,SACACB,SASACB,CASB,SASB;
	double SACASC,SACACBSC,SACACBCC,SACACC;
	double abc[3];
	double a_factor[3];
	double sinb[3];
	double cosb[3];
	double b_factor[3];
	double sinc[3];
	double cosc[3];
	double c_factor[3];
	double w,w2;
	double wsin,wcos;
	double rot0,rot1,rot3,rot4,rot6,rot7;
	double combo00,combo01,combo02,combo03,combo10,combo11,combo12,combo13,combo20,combo21,combo22,combo23;
	double combo02_2,combo02_5,combo02_8,combo02_11;
	double combo22_2,combo22_5,combo22_8,combo22_11;
	double combo12_2,combo12_5,combo12_8,combo12_11;
	const double* cpara=&(i_inst->cparam->mat[0][0]);

	factor = 10.0*MD_PI/180.0;
	//vertex展開(javaの最適化の名残)


	combo03 = cpara[0] * trans[0]+ cpara[1] * trans[1]+ cpara[2] * trans[2]+ cpara[3];
	combo13 = cpara[4] * trans[0]+ cpara[5] * trans[1]+ cpara[6] * trans[2]+ cpara[7];
	combo23 = cpara[8] * trans[0]+ cpara[9] * trans[1]+ cpara[10] * trans[2]+ cpara[11];


	nyar_NyARTransRot_O3_arGetAngle(i_inst,abc);//arGetAngle( rot, &a, &b, &c );
	a2 = abc[0];
	b2 = abc[1];
	c2 = abc[2];

	//comboの3行目を先に計算
	for(i = 0; i < 10; i++ ) {
		minerr = 1000000000.0;
		//sin-cosテーブルを計算(これが外に出せるとは…。)
		for(j=0;j<3;j++){
			w2=factor*(j-1);
			w= a2 + w2;
			a_factor[j]=w;
			w= b2 + w2;
			b_factor[j]=w;
			sinb[j]=sin(w);
			cosb[j]=cos(w);
			w= c2 + w2;
			c_factor[j]=w;
			sinc[j]=sin(w);
			cosc[j]=cos(w);
		}
		//
		for(t1=0;t1<3;t1++) {
			SA = sin(a_factor[t1]);
			CA = cos(a_factor[t1]);
			//Optimize
			CACA=CA*CA;
			SASA=SA*SA;
			SACA=SA*CA;
			for(t2=0;t2<3;t2++) {
				wsin=sinb[t2];
				wcos=cosb[t2];
				CACACB=CACA*wcos;
				SACACB=SACA*wcos;
				SASACB=SASA*wcos;
				CASB=CA*wsin;
				SASB=SA*wsin;
				//comboの計算1
				combo02 = cpara[0] * CASB+ cpara[1] * SASB+ cpara[2] * wcos;
				combo12 = cpara[4] * CASB+ cpara[5] * SASB+ cpara[6] * wcos;
				combo22 = cpara[8] * CASB+ cpara[9] * SASB+ cpara[10] * wcos;

				combo02_2 =combo02 * vertex[0][2] + combo03;
				combo02_5 =combo02 * vertex[1][2] + combo03;
				combo02_8 =combo02 * vertex[2][2] + combo03;
				combo02_11=combo02 * vertex[3][2] + combo03;
				combo12_2 =combo12 * vertex[0][2] + combo13;
				combo12_5 =combo12 * vertex[1][2] + combo13;
				combo12_8 =combo12 * vertex[2][2] + combo13;
				combo12_11=combo12 * vertex[3][2] + combo13;
				combo22_2 =combo22 * vertex[0][2] + combo23;
				combo22_5 =combo22 * vertex[1][2] + combo23;
				combo22_8 =combo22 * vertex[2][2] + combo23;
				combo22_11=combo22 * vertex[3][2] + combo23;	    
				for(t3=0;t3<3;t3++){
					wsin=sinc[t3];
					wcos=cosc[t3];			
					SACASC=SACA*wsin;
					SACACC=SACA*wcos;
					SACACBSC=SACACB*wsin;
					SACACBCC=SACACB*wcos;

					rot0 = CACACB*wcos+SASA*wcos+SACACBSC-SACASC;
					rot3 = SACACBCC-SACACC+SASACB*wsin+CACA*wsin;
					rot6 = -CASB*wcos-SASB*wsin;

					combo00 = cpara[0] * rot0+ cpara[1] * rot3+ cpara[2] * rot6;
					combo10 = cpara[4] * rot0+ cpara[5] * rot3+ cpara[6] * rot6;
					combo20 = cpara[8] * rot0+ cpara[9] * rot3+ cpara[10] * rot6;

					rot1 = -CACACB*wsin-SASA*wsin+SACACBCC-SACACC;
					rot4 = -SACACBSC+SACASC+SASACB*wcos+CACA*wcos;
					rot7 = CASB*wsin-SASB*wcos;
					combo01 = cpara[0] * rot1+ cpara[1] * rot4+ cpara[2] * rot7;
					combo11 = cpara[4] * rot1+ cpara[5] * rot4+ cpara[6] * rot7;
					combo21 = cpara[8] * rot1+ cpara[9] * rot4+ cpara[10] * rot7;
					//
					err = 0.0;
					h  = combo20 * vertex[0][0]+ combo21 * vertex[0][1]+ combo22_2;
					x = pos2d[0][0] - (combo00 * vertex[0][0]+ combo01 * vertex[0][1]+ combo02_2) / h;
					y = pos2d[0][1] - (combo10 * vertex[0][0]+ combo11 * vertex[0][1]+ combo12_2) / h;
					err += x*x+y*y;
					h  = combo20 * vertex[1][0]+ combo21 * vertex[1][1]+ combo22_5;
					x = pos2d[1][0] - (combo00 * vertex[1][0]+ combo01 * vertex[1][1]+ combo02_5) / h;
					y = pos2d[1][1] - (combo10 * vertex[1][0]+ combo11 * vertex[1][1]+ combo12_5) / h;
					err += x*x+y*y;
					h  = combo20 * vertex[2][0]+ combo21 * vertex[2][1]+ combo22_8;
					x = pos2d[2][0] - (combo00 * vertex[2][0]+ combo01 * vertex[2][1]+ combo02_8) / h;
					y = pos2d[2][1] - (combo10 * vertex[2][0]+ combo11 * vertex[2][1]+ combo12_8) / h;
					err += x*x+y*y;
					h  = combo20 * vertex[3][0]+ combo21 * vertex[3][1]+ combo22_11;
					x = pos2d[3][0] - (combo00 * vertex[3][0]+ combo01 * vertex[3][1]+ combo02_11) / h;
					y = pos2d[3][1] - (combo10 * vertex[3][0]+ combo11 * vertex[3][1]+ combo12_11) / h;
					err += x*x+y*y;
					if( err < minerr ) {
						minerr = err;
						ma = a_factor[t1];
						mb = b_factor[t2];
						mc = c_factor[t3];
						s1 = t1-1;
						s2 = t2-1;
						s3 = t3-1;
					}
				}
			}
		}
		if( s1 == 0 && s2 == 0 && s3 == 0 ){
			factor *= 0.5;
		}
		a2 = ma;
		b2 = mb;
		c2 = mc;
	}
	nyar_NyARTransRot_O3_arGetRot(ma,mb,mc,i_inst->array);
	/*  printf("factor = %10.5f\n", factor*180.0/MD_PI); */
	return minerr/4;
}

static void nyar_NyARTransRot_O3_initRotByPrevResult(
	nyar_NyARTransRot_O3_t* i_inst,
	double            i_prev_result[3][4])
{
	double* L_rot=i_inst->array;

	L_rot[0*3+0]=i_prev_result[0][0];
	L_rot[0*3+1]=i_prev_result[0][1];
	L_rot[0*3+2]=i_prev_result[0][2];

	L_rot[1*3+0]=i_prev_result[1][0];
	L_rot[1*3+1]=i_prev_result[1][1];
	L_rot[1*3+2]=i_prev_result[1][2];

	L_rot[2*3+0]=i_prev_result[2][0];
	L_rot[2*3+1]=i_prev_result[2][1];
	L_rot[2*3+2]=i_prev_result[2][2];
}
/*********************************************************************************

nyar_NyARTransMatResult擬似クラス

*********************************************************************************/


void nyar_NyARTransMatResult_updateMatrixValue(
	const double i_rot[9],
	const double i_off[3],
	const double i_trans[3],
	double o_conv[3][4])
{
	o_conv[0][0] = i_rot[0*3+0];
	o_conv[0][1] = i_rot[0*3+1];
	o_conv[0][2] = i_rot[0*3+2];
	o_conv[0][3] = i_rot[0*3+0]*i_off[0] + i_rot[0*3+1]*i_off[1] + i_rot[0*3+2]*i_off[2] + i_trans[0];

	o_conv[1][0] = i_rot[1*3+0];
	o_conv[1][1] = i_rot[1*3+1];
	o_conv[1][2] = i_rot[1*3+2];
	o_conv[1][3] = i_rot[1*3+0]*i_off[0] + i_rot[1*3+1]*i_off[1] + i_rot[1*3+2]*i_off[2] + i_trans[1];

	o_conv[2][0] = i_rot[2*3+0];
	o_conv[2][1] = i_rot[2*3+1];
	o_conv[2][2] = i_rot[2*3+2];
	o_conv[2][3] = i_rot[2*3+0]*i_off[0] + i_rot[2*3+1]*i_off[1] + i_rot[2*3+2]*i_off[2] + i_trans[2];
	return;
}

/*********************************************************************************

nyar_NyARTransMat_O2擬似クラス

*********************************************************************************/

static void nyar_NyARTransMat_O2_initPos3d(
	const double* i_ppos3d,
	double* o_pos3d,
	double o_off[]);

static double nyar_NyARTransMat_O2_arGetTransMatSub(
	const nyar_NyARTransMat_O2_t* i_inst,
	double            i_pos2d[][2],
	double            i_pos3d[][3],
	ARMat*                  i_mat_b,
	ARMat*                  i_mat_d,
	double                  o_trans[]);

static void nyar_NyARTransMat_O2_arGetTransMat3_initTransMat(
	nyar_NyARTransMat_O2_t* i_inst,
	double i_ppos2d[][2],
	double i_ppos3d[][2],
	double o_pos2d[][2],
	double o_pos3d[][3],
	double o_off[3],
	ARMat* o_mat_b,
	ARMat* o_mat_d);


#define AR_GET_TRANS_MAT_MAX_LOOP_COUNT 5
#define AR_GET_TRANS_MAT_MAX_FIT_ERROR 1.0
#define P_MAX 4
#define NUMBER_OF_VERTEX 4


struct nyar_NyARTransMat_O2{
	nyar_NyARTransRot_O3_t* transrot;
	const ARParam* param;
	struct{
		ARMat* transMat_mat_a;
		ARMat* transMat_mat_b;
		ARMat* transMat_mat_c;
		ARMat* transMat_mat_d;
		ARMat* transMat_mat_e;
		ARMat* transMat_mat_f;
	}wk;
};

nyar_NyARTransMat_O2_t* nyar_NyARTransMat_O2_create(
	const ARParam* i_param)
{
	nyar_NyARTransMat_O2_t* inst;
	inst=(nyar_NyARTransMat_O2_t*)malloc(sizeof(nyar_NyARTransMat_O2_t));
	inst->transrot=nyar_NyARTransRot_O3_create(i_param,4);
	inst->param=i_param;
	inst->wk.transMat_mat_a=arMatrixAlloc(NUMBER_OF_VERTEX*2,3);
	inst->wk.transMat_mat_b=arMatrixAlloc(3,NUMBER_OF_VERTEX*2);
	inst->wk.transMat_mat_c=arMatrixAlloc(NUMBER_OF_VERTEX*2,1);
	inst->wk.transMat_mat_d=arMatrixAlloc(3,3);
	inst->wk.transMat_mat_e=arMatrixAlloc(3,1);
	inst->wk.transMat_mat_f=arMatrixAlloc(3,1);
	return inst;
}
void nyar_NyARTransMat_O2_free(
	nyar_NyARTransMat_O2_t* i_inst)
{
	arMatrixFree(i_inst->wk.transMat_mat_a);
	arMatrixFree(i_inst->wk.transMat_mat_b);
	arMatrixFree(i_inst->wk.transMat_mat_c);
	arMatrixFree(i_inst->wk.transMat_mat_d);
	arMatrixFree(i_inst->wk.transMat_mat_e);
	arMatrixFree(i_inst->wk.transMat_mat_f);
	nyar_NyARTransRot_O3_free(i_inst->transrot);
	free(i_inst);
}




static void nyar_NyARTransMat_O2_init_transMat_ppos(
	const ARMarkerInfo* i_marker_info,
	const double  i_width,
	const double* i_center,
	double o_ppos2d[4][2],
	double o_ppos3d[4][2])
{	
	double c0,c1,w_2;
	int dir=i_marker_info->dir;
	o_ppos2d[0][0] = i_marker_info->vertex[(4-dir)%4][0];
	o_ppos2d[0][1] = i_marker_info->vertex[(4-dir)%4][1];
	o_ppos2d[1][0] = i_marker_info->vertex[(5-dir)%4][0];
	o_ppos2d[1][1] = i_marker_info->vertex[(5-dir)%4][1];
	o_ppos2d[2][0] = i_marker_info->vertex[(6-dir)%4][0];
	o_ppos2d[2][1] = i_marker_info->vertex[(6-dir)%4][1];
	o_ppos2d[3][0] = i_marker_info->vertex[(7-dir)%4][0];
	o_ppos2d[3][1] = i_marker_info->vertex[(7-dir)%4][1];


	c0=i_center[0];
	c1=i_center[1];
	w_2 =i_width/2.0;

	o_ppos3d[0][0] = c0 - w_2;//center[0] - w/2.0;
	o_ppos3d[0][1] = c1 + w_2;//center[1] + w/2.0;
	o_ppos3d[1][0] = c0 + w_2;//center[0] + w/2.0;
	o_ppos3d[1][1] = c1 + w_2;//center[1] + w/2.0;
	o_ppos3d[2][0] = c0 + w_2;//center[0] + w/2.0;
	o_ppos3d[2][1] = c1 - w_2;//center[1] - w/2.0;
	o_ppos3d[3][0] = c0 - w_2;//center[0] - w/2.0;
	o_ppos3d[3][1] = c1 - w_2;//center[1] - w/2.0;
	return;
}

double nyar_NyARTransMat_O2_transMat(
	nyar_NyARTransMat_O2_t* i_inst,
	ARMarkerInfo* i_marker_info,
	double i_center[2],
	double i_width,
	double o_conv[3][4])
{
	double ppos2d[4][2];
	double ppos3d[4][2];
	double off[3];
	double pos3d[P_MAX][3];
	double pos2d[P_MAX][2];


	int i;
	/**/

	ARMat* mat_b =i_inst->wk.transMat_mat_b;
	ARMat* mat_d =i_inst->wk.transMat_mat_d;
	/**/
	double  err=-1;

	double trans[3];


	//rotationの初期化
	nyar_NyARTransRot_O3_arGetInitRot(i_inst->transrot,i_marker_info);

	//ppos2dとppos3dの初期化
	nyar_NyARTransMat_O2_init_transMat_ppos(i_marker_info,i_width,i_center,ppos2d,ppos3d);

	//arGetTransMat3の前段処理(pos3dとoffを初期化)
	nyar_NyARTransMat_O2_arGetTransMat3_initTransMat(i_inst,ppos2d,ppos3d,pos2d,pos3d,off,mat_b,mat_d);	

	for(i=0;i<AR_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ){
		//<arGetTransMat3>
		err = nyar_NyARTransMat_O2_arGetTransMatSub(i_inst,pos2d,pos3d,mat_b,mat_d,trans);
	//	    //</arGetTransMat3>
		if( err < AR_GET_TRANS_MAT_MAX_FIT_ERROR ){
			break;
		}
	}
	//マトリクスの保存
	nyar_NyARTransMatResult_updateMatrixValue(i_inst->transrot->array,off,trans,o_conv);
	return err;
}

double nyar_NyARTransMat_O2_transMatCont(
	nyar_NyARTransMat_O2_t* i_inst,
	ARMarkerInfo*     i_marker_info,
	double            i_prev_conv[3][4],
	double            i_center[2],
	double            i_width,
	double            o_conv[3][4])
/*
double nyar_NyARTransMat_O2_transMatCont(
	NyARSquare i_square,int i_direction, double i_width,NyARTransMatResult io_result_conv)throws NyARException*/
{
	double ppos2d[4][2];
	double ppos3d[4][2];
	double pos3d[P_MAX][3];
	double pos2d[P_MAX][2];
	double off[3];

	ARMat* mat_b =i_inst->wk.transMat_mat_b;
	ARMat* mat_d =i_inst->wk.transMat_mat_d;
	double  err1,err2;
	int     i;
	double trans[3];
	double conv2[3][4];

//	double[][]  ppos2d=wk_transMat_ppos2d;
//	double[][]  ppos3d=wk_transMat_ppos3d;
//	double[]    off   =wk_transMat_off;
//	double[][]  pos3d =wk_transMat_pos3d;
	
	//	arGetTransMatContSub計算部分
	nyar_NyARTransRot_O3_initRotByPrevResult(i_inst->transrot,i_prev_conv);

	//ppos2dとppos3dの初期化
	nyar_NyARTransMat_O2_init_transMat_ppos(i_marker_info,i_width,i_center,ppos2d,ppos3d);
	
	//arGetTransMat3の前段処理(pos3dとoffを初期化)
	//	double[][] pos2d=this.wk_transMat_pos2d;


	//transMatに必要な初期値を計算
	nyar_NyARTransMat_O2_arGetTransMat3_initTransMat(i_inst,ppos2d,ppos3d,pos2d,pos3d,off,mat_b,mat_d);	

	err1=err2=-1;
	for( i = 0; i < AR_GET_TRANS_MAT_MAX_LOOP_COUNT; i++ ){
	    err1 = nyar_NyARTransMat_O2_arGetTransMatSub(i_inst,pos2d,pos3d,mat_b,mat_d,trans);
	    if( err1 < AR_GET_TRANS_MAT_MAX_FIT_ERROR ){
			//十分な精度を達成できたらブレーク
			break;
	    }
	}
	//値を保存
	nyar_NyARTransMatResult_updateMatrixValue(i_inst->transrot->array,off,trans,o_conv);

	//エラー値が許容範囲でなければTransMatをやり直し
	if(err1>AR_GET_TRANS_CONT_MAT_MAX_FIT_ERROR){
	    //transMatを実行(初期化値は共用)
	    nyar_NyARTransRot_O3_arGetInitRot(i_inst->transrot,i_marker_info);
		err2=nyar_NyARTransMat_O2_transMat(i_inst,i_marker_info,i_center,i_width,conv2);
	    //transmMatここまで
	    if(err2<err1){
			//良い値が取れたら、差換え
			memcpy(o_conv,conv2,sizeof(double)*3*4);
			err1 = err2;
	    }
	}
	return err1;
}


/*	Java版の不具合でi_ppos2dとi_ppos3dの順番がひっくり返ってたのを直してある。


*/
static void nyar_NyARTransMat_O2_arGetTransMat3_initTransMat(
	nyar_NyARTransMat_O2_t* i_inst,
	double i_ppos2d[][2],
	double i_ppos3d[][2],
	double o_pos2d[][2],
	double o_pos3d[][3],
	double o_off[3],
	ARMat* o_mat_b,
	ARMat* o_mat_d)
{
	double pmax0,pmax1,pmax2,pmin0,pmin1,pmin2;
	int i;
	ARMat* mat_a =i_inst->wk.transMat_mat_a;
	const double* cpara=&(i_inst->param->mat[0][0]);
	int x2;

	pmax0=pmax1=pmax2 = -10000000000.0;
	pmin0=pmin1=pmin2 =  10000000000.0;
	for(i = 0; i < NUMBER_OF_VERTEX; i++ ) {
	    if( i_ppos3d[i][0] > pmax0 ){
			pmax0 = i_ppos3d[i][0];
	    }
	    if( i_ppos3d[i][0] < pmin0 ){
			pmin0 = i_ppos3d[i][0];
	    }
	    if( i_ppos3d[i][1] > pmax1 ){
			pmax1 = i_ppos3d[i][1];
	    }
	    if( i_ppos3d[i][1] < pmin1 ){
			pmin1 = i_ppos3d[i][1];
	    }
	    /*	オリジナルでもコメントアウト
	        if( ppos3d[i][2] > pmax[2] ) pmax[2] = ppos3d[i][2];
	        if( ppos3d[i][2] < pmin[2] ) pmin[2] = ppos3d[i][2];
	     */	 
	}
	o_off[0] = -(pmax0 + pmin0) / 2.0;
	o_off[1] = -(pmax1 + pmin1) / 2.0;
	o_off[2] = -(pmax2 + pmin2) / 2.0;


	for(i = 0; i < NUMBER_OF_VERTEX; i++ ) {
	    o_pos3d[i][0] = i_ppos3d[i][0] + o_off[0];
	    o_pos3d[i][1] = i_ppos3d[i][1] + o_off[1];
	    o_pos3d[i][2] = 0.0;
	}
	//ココから先でarGetTransMatSubの初期化処理
	//arGetTransMatSubにあった処理。毎回おなじっぽい。pos2dに変換座標を格納する。
	
	if(arFittingMode == AR_FITTING_TO_INPUT ){
		for(i = 0; i < NUMBER_OF_VERTEX; i++ ) {
			arParamIdeal2Observ(i_inst->param->dist_factor,i_ppos2d[i][0],i_ppos2d[i][1],&o_pos2d[i][0],&o_pos2d[i][1]);
		}
	}else{
	    for(i = 0; i < NUMBER_OF_VERTEX; i++ ){
			o_pos2d[i][0] = i_ppos2d[i][0];
			o_pos2d[i][1] = i_ppos2d[i][1];
	    }
	}


	for(i = 0; i < NUMBER_OF_VERTEX; i++ ) {
	    x2=i*2;
	    //</Optimize>
		mat_a->m[x2*3+0]=o_mat_b->m[0*NUMBER_OF_VERTEX*2+x2]=cpara[0*4+0];//mat_a->m[j*6+0] = mat_b->m[num*0+j*2] = cpara[0][0];
		mat_a->m[x2*3+1]=o_mat_b->m[1*NUMBER_OF_VERTEX*2+x2]=cpara[0*4+1];//mat_a->m[j*6+1] = mat_b->m[num*2+j*2] = cpara[0][1];
		mat_a->m[x2*3+2]=o_mat_b->m[2*NUMBER_OF_VERTEX*2+x2]=cpara[0*4+2]-o_pos2d[i][0];//mat_a->m[j*6+2] = mat_b->m[num*4+j*2] = cpara[0][2] - pos2d[j][0];
		mat_a->m[(x2+1)*3+0]=o_mat_b->m[0*NUMBER_OF_VERTEX*2+x2+1]=0.0;//mat_a->m[j*6+3] = mat_b->m[num*0+j*2+1] = 0.0;
		mat_a->m[(x2+1)*3+1]=o_mat_b->m[1*NUMBER_OF_VERTEX*2+x2+1]=cpara[1*4+1];//mat_a->m[j*6+4] = mat_b->m[num*2+j*2+1] = cpara[1][1];
		mat_a->m[(x2+1)*3+2]=o_mat_b->m[2*NUMBER_OF_VERTEX*2+x2+1]=cpara[1*4+2] - o_pos2d[i][1];//mat_a->m[j*6+5] = mat_b->m[num*4+j*2+1] = cpara[1][2] - pos2d[j][1];
	}
	
	//mat_d
	arMatrixMul(o_mat_d,o_mat_b,mat_a);
	arMatrixSelfInv(o_mat_d);
}

static double nyar_NyARTransMat_O2_arGetTransMatSub(
	const nyar_NyARTransMat_O2_t* i_inst,
	double            i_pos2d[][2],
	double            i_pos3d[][3],
	ARMat*                  i_mat_b,
	ARMat*                  i_mat_d,
	double                  o_trans[])
{
	ARMat* mat_c=i_inst->wk.transMat_mat_c;
	ARMat* mat_e=i_inst->wk.transMat_mat_e;
	ARMat* mat_f=i_inst->wk.transMat_mat_f;
	const double* cpara=&(i_inst->param->mat[0][0]);
	double  wx, wy, wz;
	double  ret;
	int     i;
	double* rot=i_inst->transrot->array;

	int x2;
	for( i = 0; i < NUMBER_OF_VERTEX; i++ ) {
	    x2=i*2;
	    wx = rot[0] * i_pos3d[i][0]+ rot[1] * i_pos3d[i][1]+ rot[2] * i_pos3d[i][2];
	    wy = rot[3] * i_pos3d[i][0]+ rot[4] * i_pos3d[i][1]+ rot[5] * i_pos3d[i][2];
	    wz = rot[6] * i_pos3d[i][0]+ rot[7] * i_pos3d[i][1]+ rot[8] * i_pos3d[i][2];
		mat_c->m[x2+0]=wz * i_pos2d[i][0]- cpara[0*4+0]*wx - cpara[0*4+1]*wy - cpara[0*4+2]*wz;//mat_c->m[j*2+0] = wz * pos2d[j][0]- cpara[0][0]*wx - cpara[0][1]*wy - cpara[0][2]*wz;
	    mat_c->m[x2+1]=wz * i_pos2d[i][1]- cpara[1*4+1]*wy - cpara[1*4+2]*wz;//mat_c->m[j*2+1] = wz * pos2d[j][1]- cpara[1][1]*wy - cpara[1][2]*wz;
	}

	arMatrixMul(mat_e,i_mat_b, mat_c );
	arMatrixMul(mat_f,i_mat_d, mat_e );
	o_trans[0] = mat_f->m[0];//trans[0] = mat_f->m[0];
	o_trans[1] = mat_f->m[1];
	o_trans[2] = mat_f->m[2];//trans[2] = mat_f->m[2];

	ret =nyar_NyARTransRot_O3_modifyMatrix(i_inst->transrot,o_trans, i_pos3d, i_pos2d);
	for( i = 0; i < NUMBER_OF_VERTEX; i++ ) {
	    x2=i*2;
	    wx = rot[0] * i_pos3d[i][0]+ rot[1] * i_pos3d[i][1]+ rot[2] * i_pos3d[i][2];
	    wy = rot[3] * i_pos3d[i][0]+ rot[4] * i_pos3d[i][1]+ rot[5] * i_pos3d[i][2];
	    wz = rot[6] * i_pos3d[i][0]+ rot[7] * i_pos3d[i][1]+ rot[8] * i_pos3d[i][2];
	    mat_c->m[x2+0]=wz * i_pos2d[i][0]- cpara[0*4+0]*wx - cpara[0*4+1]*wy - cpara[0*4+2]*wz;//mat_c->m[j*2+0] = wz * pos2d[j][0]- cpara[0][0]*wx - cpara[0][1]*wy - cpara[0][2]*wz;
	    mat_c->m[x2+1]=wz * i_pos2d[i][1]- cpara[1*4+1]*wy - cpara[1*4+2]*wz;//mat_c->m[j*2+1] = wz * pos2d[j][1]- cpara[1][1]*wy - cpara[1][2]*wz;
	}

	arMatrixMul(mat_e,i_mat_b, mat_c );
	arMatrixMul(mat_f,i_mat_d, mat_e );
	o_trans[0] = mat_f->m[0];//trans[0] = mat_f->m[0];
	o_trans[1] = mat_f->m[1];
	o_trans[2] = mat_f->m[2];//trans[2] = mat_f->m[2];
	ret =nyar_NyARTransRot_O3_modifyMatrix(i_inst->transrot,o_trans, i_pos3d, i_pos2d);
	return ret;
}


