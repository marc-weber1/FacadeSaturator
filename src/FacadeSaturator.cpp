#include "FacadeSaturator.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include <iostream>
#include <fstream>

const char* PARAM_NAMES[] = { "Remove DC Offset","CurvePoint1-X","CurvePoint1-Y","CurvePoint2-X","CurvePoint2-Y","CurvePoint3-X","CurvePoint3-Y","CurvePoint4-X","CurvePoint4-Y","CurvePoint5-X","CurvePoint5-Y","CurvePoint6-X","CurvePoint6-Y","CurvePoint7-X","CurvePoint7-Y","CurvePoint8-X","CurvePoint8-Y","CurvePoint9-X","CurvePoint9-Y","CurvePoint10-X","CurvePoint10-Y","CurvePoint11-X","CurvePoint11-Y","CurvePoint12-X","CurvePoint12-Y","CurvePoint13-X","CurvePoint13-Y","CurvePoint14-X","CurvePoint14-Y","CurvePoint15-X","CurvePoint15-Y","CurvePoint16-X","CurvePoint16-Y","ShapePoint1-X","ShapePoint1-Y","ShapePoint2-X","ShapePoint2-Y","ShapePoint3-X","ShapePoint3-Y","ShapePoint4-X","ShapePoint4-Y","ShapePoint5-X","ShapePoint5-Y","ShapePoint6-X","ShapePoint6-Y","ShapePoint7-X","ShapePoint7-Y","ShapePoint8-X","ShapePoint8-Y","ShapePoint9-X","ShapePoint9-Y","ShapePoint10-X","ShapePoint10-Y","ShapePoint11-X","ShapePoint11-Y","ShapePoint12-X","ShapePoint12-Y","ShapePoint13-X","ShapePoint13-Y","ShapePoint14-X","ShapePoint14-Y","ShapePoint15-X","ShapePoint15-Y","ShapePoint16-X","ShapePoint16-Y","Num Points" };


FacadeSaturator::FacadeSaturator(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms)), curve(this, kRemoveDCOffset, kInitCurvePoint, kInitShapePoint, kNumPointsEnabled)
{
	GetParam(kRemoveDCOffset)->InitBool(PARAM_NAMES[kRemoveDCOffset], true);
	
	GetParam(kInitCurvePoint+0)->InitDouble(PARAM_NAMES[kInitCurvePoint+0], -1., -1., 1., 0.001, "");
	GetParam(kInitCurvePoint+1)->InitDouble(PARAM_NAMES[kInitCurvePoint+1], -1., -1., 1., 0.001, "");
	GetParam(kInitCurvePoint+2)->InitDouble(PARAM_NAMES[kInitCurvePoint+2], 1., -1., 1., 0.001, "");
	GetParam(kInitCurvePoint+3)->InitDouble(PARAM_NAMES[kInitCurvePoint+3], 1., -1., 1., 0.001, "");
	GetParam(kInitShapePoint+0)->InitDouble(PARAM_NAMES[kInitShapePoint+0], 0.1, 0., 2., 0.001, "");
	GetParam(kInitShapePoint+1)->InitDouble(PARAM_NAMES[kInitShapePoint+1], 0.05, -2., 2., 0.001, "");
	GetParam(kInitShapePoint+2)->InitDouble(PARAM_NAMES[kInitShapePoint+2], 0.1, 0., 2., 0.001, "");
	GetParam(kInitShapePoint+3)->InitDouble(PARAM_NAMES[kInitShapePoint+3], 0.05, -2., 2., 0.001, "");
	
	for(int i=2;i<kNumPoints-1;i++){
		GetParam(kInitCurvePoint+2*i)->InitDouble(PARAM_NAMES[kInitCurvePoint+2*i], 0., -1., 1., 0.001, "");
		GetParam(kInitCurvePoint+2*i+1)->InitDouble(PARAM_NAMES[kInitCurvePoint+2*i+1], 0., -1., 1., 0.001, "");
		GetParam(kInitShapePoint+2*i)->InitDouble(PARAM_NAMES[kInitShapePoint+2*i], 0.1, 0., 2., 0.001, "");
		GetParam(kInitShapePoint+2*i+1)->InitDouble(PARAM_NAMES[kInitShapePoint+2*i+1], 0.05, -2., 2., 0.001, "");
	}
	GetParam(kNumPointsEnabled)->InitInt(PARAM_NAMES[kNumPointsEnabled], 2, 2, kNumPoints);
}

#if IPLUG_DSP
void FacadeSaturator::OnActivate(bool enable){ // When the DSP turns on?
	curve.update_params_from_host();
}

void FacadeSaturator::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	const int nChans = NOutChansConnected();
  
	for (int c = 0; c < nChans; c++) {
		curve.waveshape(inputs[c], outputs[c], nFrames, c == nChans-1);
		float_clamp(outputs[c],outputs[c],nFrames,-1.f,1.f); //Clamp into [-1,1] with hard clipping
	}
	
	//Copy values to oscilloscope
	if(mWindow){
		for(int c=0; c<2 && c<nChans; c++){
			oscilloscopeBuffer[c].Add((float*) outputs[c],nFrames);
		}
	}
}
#endif



// UI INTERFACE

#if IPLUG_EDITOR // http://bit.ly/2S64BDd

void* FacadeSaturator::OpenWindow(void* pParent){
	
	void* ret_ptr = nullptr;
	
	if(!mWindow){
		mUI = std::unique_ptr<TestUI>(new TestUI((IEditorDelegate*) this,&curve,oscilloscopeBuffer,NOutChansConnected()>2 ? 2 : NOutChansConnected() ));
		mWindow = std::unique_ptr<GL3PluginWindow>(new GL3PluginWindow((HWND) pParent,PLUG_WIDTH,PLUG_HEIGHT,PLUG_FPS,1.f, //Fix the HWND when porting
		{4,1,-1,false,false,false,false,false,false},
		(GL3PluginUI*) mUI.get())); 
		if(mLastWidth && mLastHeight && mLastScale){
			mWindow->Resize(mLastWidth, mLastHeight, mLastScale);
		}
	}
	
	if(mWindow) ret_ptr = mWindow->OpenWindow(pParent);
	
	curve.update_params_from_host();
	
	return ret_ptr;
}

void FacadeSaturator::CloseWindow(){
	IEditorDelegate::CloseWindow(); //necessary??
	
	mWindow->CloseWindow();
	mWindow = nullptr;
	mUI = nullptr;
}

void FacadeSaturator::OnParamChangeUI(int paramIdx, EParamSource source){
	if(source == kDelegate) curve.update_param_from_host(paramIdx);
}

#endif