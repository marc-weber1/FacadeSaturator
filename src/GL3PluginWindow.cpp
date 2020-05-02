#include "GL3PluginWindow.h"

#include <stdio.h>
#include <math.h>


static PuglStatus onEvent(PuglView* view, const PuglEvent* event){
	GL3PluginWindow* app = (GL3PluginWindow*)puglGetHandle(view);
	return app->handleEvent(event);
}

static void puglLog(PuglWorld* world, PuglLogLevel level, const char* msg){
	printf("%s\n",msg);
}


GL3PluginWindow::GL3PluginWindow(void* pParent,unsigned w, unsigned h, unsigned fps, float scale, const PuglOptions opts, std::unique_ptr<GL3PluginUI>& ui_t): m_width(w), m_height(h), m_fps(fps), m_scale(scale), m_opts(opts), ui(ui_t){}

PuglStatus GL3PluginWindow::handleEvent(const PuglEvent* event){
	switch(event->type){
		case PUGL_CONFIGURE:
			(void)view;
			glClearColor(0.f,0.f,0.f,1.f);
			glViewport(0,0,(int)event->configure.width,(int)event->configure.height);
			break;
		case PUGL_EXPOSE:
			ui->drawFrame(view);
			break;
		case PUGL_TIMER:
			if(event->timer.id == GL3PluginWindow::REDRAW_TIMER_ID){
				puglPostRedisplay(view);
			}
			break;
		case PUGL_BUTTON_PRESS:
			ui->mouseDown(event->button.button,event->button.x/m_width*2-1,event->button.y/m_height*-2+1);
			break;
		case PUGL_BUTTON_RELEASE:
			ui->mouseUp(event->button.button,event->button.x/m_width*2-1,event->button.y/m_height*-2+1);
			break;
		case PUGL_MOTION:
			ui->mouseMove(event->button.x/m_width*2-1,event->button.y/m_height*-2+1);
			break;
		case PUGL_CREATE:
			// Load GL functions via GLAD (DOESNT WORK ON MAC !!)
			if (!gladLoadGLLoader((GLADloadproc)&puglGetProcAddress)) {
				printf("Failed to load GL\n");
				return PUGL_FAILURE;
			}
			printf("Loaded GL successfully.\n");
			ui->initGLContext();
			break;
		case PUGL_DESTROY:
			ui->destroyGLContext();
			break;
	}
	
	return PUGL_SUCCESS;
}

void GL3PluginWindow::Resize(int w,int h,float scale){
	
}

void* GL3PluginWindow::OpenWindow(void* pParent){
	world = puglNewWorld(PUGL_PROGRAM, 0);

	// Set up world
	puglSetClassName(world, "GL3PluginWindow");
	puglSetLogFunc(world, puglLog);
	puglSetLogLevel(world, PUGL_LOG_LEVEL_DEBUG);
	
	double startpos[2] = {0.,0.};
	if(pParent == nullptr){ //For debugging
		startpos[0]=300.;
		startpos[1]=300.;
	}
	const PuglRect frame = {startpos[0], startpos[1], (double) m_width, (double) m_height};
	
	view = puglNewView(world);
	
	puglSetParentWindow(view, (PuglNativeWindow) pParent);
	puglSetFrame(view, frame);
	puglSetMinSize(view, m_width, m_height);
	puglSetAspectRatio(view, 1, 1, 16, 9); //min aspect x y, max aspect x y
	puglSetBackend(view, puglGlBackend());
	puglSetViewHint(view, PUGL_USE_COMPAT_PROFILE, PUGL_FALSE);
	puglSetViewHint(view, PUGL_USE_DEBUG_CONTEXT, m_opts.errorChecking);
	puglSetViewHint(view, PUGL_CONTEXT_VERSION_MAJOR, 3);
	puglSetViewHint(view, PUGL_CONTEXT_VERSION_MINOR, 3);
	puglSetViewHint(view, PUGL_RESIZABLE, m_opts.resizable);
	puglSetViewHint(view, PUGL_SAMPLES, m_opts.samples);
	puglSetViewHint(view, PUGL_DOUBLE_BUFFER, m_opts.doubleBuffer);
	puglSetViewHint(view, PUGL_SWAP_INTERVAL, m_opts.sync);
	puglSetViewHint(view, PUGL_IGNORE_KEY_REPEAT, PUGL_TRUE);
	puglSetHandle(view, static_cast<GL3PluginWindow*>(this) );
	puglSetEventFunc(view, onEvent);
	
	//Create the window
	const PuglStatus st = puglCreateWindow(view, WINDOW_NAME);
	if(st){
		//ERROR
		printf("ERROR\n");
		return nullptr;
	}
	
	puglShowWindow(view);
	
	puglStartTimer(view,REDRAW_TIMER_ID,0.015);
	
	return (void*) puglGetNativeWindow(view);
}

void GL3PluginWindow::CloseWindow(){
	puglStopTimer(view,REDRAW_TIMER_ID);
	puglFreeView(view);
	puglFreeWorld(world);
}

void GL3PluginWindow::ForceCheckEvents(){
	puglDispatchEvents(world);
}