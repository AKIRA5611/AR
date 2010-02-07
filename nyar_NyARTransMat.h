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
#ifndef nyar_NyARTransMat_h
#define nyar_NyARTransMat_h

#define nyar_AR_FITTING_TO_IDEAL 0
#define nyar_AR_FITTING_TO_INPUT 1
#define nyar_FittingMode AR_FITTING_TO_INPUT

#ifdef __cplusplus
extern "C" {
#endif


typedef struct nyar_NyARTransMat_O2 nyar_NyARTransMat_O2_t;

/*!
		nyar_NyARTransMat_O2�C���X�^���X�̃R���X�g���N�^�ł��B
	\param i_param
		�J�����p�����[�^�̎Q�ƒl��n���܂��B
		���̎Q�ƒl�́Anyar_NyARTransMat_O2_free�ŃC���X�^���X���J������܂�
		�ێ����Ă��������B
	\return
		�������ς݂�nyar_NyARTransMat_O2�C���X�^���X��Ԃ��܂��B
*/
nyar_NyARTransMat_O2_t* nyar_NyARTransMat_O2_create(
	const ARParam* i_param);

/*!
		nyar_NyARTransMat_O2�C���X�^���X�̃f�X�g���N�^�ł��B
	\param i_inst
		�J������nyar_NyARTransMat_O2�C���X�^���X��n���܂��B
*/
void nyar_NyARTransMat_O2_free(
	nyar_NyARTransMat_O2_t* i_inst);

/*!
		arGetTransMat�݊��̊֐��ł��B
	\param i_inst
		nyar_NyARTransMat_O2�C���X�^���X��n���܂��B
		���̃C���X�^���X�́Anyar_NyARTransMat_O2_create�֐��Ő����������̂ł���K�v������܂��B
	\param i_marker_info
		arGetTransMat�Ɠ����ł��B
	\param center
		arGetTransMat�Ɠ����ł��B
	\param i_width
		arGetTransMat�Ɠ����ł��B
	\param o_conv
		arGetTransMat�Ɠ����ł��B
	\return
		arGetTransMat�Ɠ����ł��B		
*/
double nyar_NyARTransMat_O2_transMat(
	nyar_NyARTransMat_O2_t* i_inst,
	ARMarkerInfo* i_marker_info,
	double center[2],
	double i_width,
	double o_conv[3][4]);

/*!
		arGetTransMatCont�݊��̊֐��ł��B
	\param i_inst
		nyar_NyARTransMat_O2�C���X�^���X��n���܂��B
		���̃C���X�^���X�́Anyar_NyARTransMat_O2_create�֐��Ő����������̂ł���K�v������܂��B
	\param i_marker_info
		arGetTransMat�Ɠ����ł��B
	\param i_prev_conv
		arGetTransMat�Ɠ����ł��B
	\param i_center
		arGetTransMat�Ɠ����ł��B
	\param i_width
		arGetTransMat�Ɠ����ł��B
	\param o_conv
		arGetTransMat�Ɠ����ł��B
	\return
		arGetTransMat�Ɠ����ł��B		
*/
double nyar_NyARTransMat_O2_transMatCont(
	nyar_NyARTransMat_O2_t* i_inst,
	ARMarkerInfo*     i_marker_info,
	double            i_prev_conv[3][4],
	double            i_center[2],
	double            i_width,
	double            o_conv[3][4]);

#ifdef __cplusplus
}
#endif

#endif
