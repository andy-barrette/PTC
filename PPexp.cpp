#include "audioconfig.h"
#include "GUItools01.h"
using namespace std;

#define TWOPI 6.28318531

audiohwtype ahw;
pagetype *curpage;
int progstate;
pagetype homepage,newtypepage,newptcpage,loadpage,ptcsetpage,instructpage,soundpage,questionpage,resultspage;
dropdowntype ptctypedd,trtypedd;
int curtime;

class swave
{
	public:
	float f;//frequency
	float a;//amplitude
	float p;//phase displacement
	
	void set(float amp,float freq,float phase)
	{
		a=amp;
		f=freq;
		p=phase;
	}	
	int createsamples(short *buffer,float offset,float length,float gain)//offset and length in seconds
	{
		int i;
		for(i=offset*ahw.rate;i<(offset+length)*ahw.rate;i++)
			buffer[i]+=(short)(gain*a*sin(p+f*(float)i*TWOPI/(float)ahw.rate));
	}	
};

class sound
{
	public:
	vector<swave> comp;
	
	int createsamples(short *buffer,float offset,float length,float gain)//offset and length in seconds
	{
		int i,c;

		for(c=0;c<comp.size();c++)
			comp[c].createsamples(buffer,offset,length,gain);
	}
};

class noisetype
{
	public:
	float frange[2];//inner and outer freq boundaries of swave components of noise
	float ffreq;//frequency of swave component frequencies
	float a;//amplitude;
	void set(float amp,float f1,float f2,float ff)
	{
		a=amp;
		frange[0]=f1;
		frange[1]=f2;
		ffreq=ff;
	}

	int createsamples(short *buffer,float offset,float length,float gain)//offset and length in seconds
	{
		int i;
		float fcur,p;

		if(frange[0]<frange[1])
			for(fcur=frange[0];fcur<=frange[1];fcur+=ffreq)
			{
				p=rand();
				for(i=offset*ahw.rate;i<(offset+length)*ahw.rate;i++)
					buffer[i]+=(short)(gain*a*sin(p+fcur*(float)i*TWOPI/(float)ahw.rate));
			}
		else if(frange[1]<frange[0])
			for(fcur=frange[0];fcur>=frange[1];fcur-=ffreq)
			{
				p=rand();
				for(i=offset*ahw.rate;i<(offset+length)*ahw.rate;i++)
					buffer[i]+=(short)(gain*a*sin(p+fcur*(float)i*TWOPI/(float)ahw.rate));
			}
	}
};

void monotostereo(short *buffer,int frames)
{
	int i;
	for(i=frames-1;i>=0;i--)
	{
		buffer[2*i+1]=buffer[2*i]=buffer[i];
	}
}
void fadein(short *buffer,float start,float length,int type)//type(1=linear,2=exponential,3=sinusoidal),start and length are in seconds
{
	int i;
	for(i=0;i<start*ahw.rate;i++)
		buffer[i]=(short)0;
	for(i=start*ahw.rate;i<(start+length)*ahw.rate;i++)
		buffer[i]=(short)((float)buffer[i]*(((float)i/(float)ahw.rate)-start)/length);
}
void fadeout(short *buffer,int frames,float end,float length,int type)//type(1=linear,2=exponential,3=sinusoidal),start and length are in seconds
{
	int i;
	for(i=(end-length)*ahw.rate;i<=end*ahw.rate;i++)
		buffer[i]=(short)((float)buffer[i]*(end-((float)i/(float)ahw.rate))/length);
	for(i=end*ahw.rate;i<frames;i++)
		buffer[i]=(short)0;
}

class ptcexptype
{
	public:
	short curtrial,cursubtrial;
	short type;//1 for asymmetric noise, 2 for symmetric noise
	noisetype n1,n2;
	swave tone;
	float length;//in seconds
	float *indvar;//independent variable: this is the variable that will change with each trial
	short changetype;//1=linear,2=geometric,3=exponential
	float changefact;//change factor
	short trialcount;
	string question;//question with which subject is prompted after each trial's audio is played
	vector<float> indvarvals;
	vector<float> results;
	short lastresponse;
	float gain,startingamp;
	
	void runtrial(short num)
	{
		if(num>0)
			switch(changetype)
			{
				case 0:
					*indvar+=changefact;
					break;
				case 1:
					*indvar*=changefact;
					break;
				case 2:
					*indvar=pow(*indvar,changefact);
					break;
			}
		indvarvals.push_back(*indvar);
		cursubtrial=0;
		tone.a=startingamp;
		lastresponse=1;
		instructpage.buttons[0].enabled=false;
		curpage=&instructpage;
		instructpage.enable();
	}
	short runsubtrial()
	{
		instructpage.buttons[0].enabled=false;
		short *buffer;
		int err;
		buffer=(short*)malloc(4*length*ahw.rate);
		for(int i=0;i<2*length*ahw.rate;i++)
			buffer[i]=(short)0;
		float temp=(float)(rand()%10)/20.0;
		tone.createsamples(buffer,temp+length/2.0,.25,gain);
		fadein(buffer,temp+length/2.0,.1,1);
		fadeout(buffer,2*length*ahw.rate,temp+length/2.0+.25,.1,1);
		n1.createsamples(buffer,0,length,gain);
		n2.createsamples(buffer,0,length,gain);
		fadein(buffer,0,.2,1);
		fadeout(buffer,2*length*ahw.rate,length,.2,1);
		monotostereo(buffer,length*ahw.rate);
		if((err=snd_pcm_open(&ahw.pcmhandle,"hw:0,0",SND_PCM_STREAM_PLAYBACK,0))<0)
		{
			printf("\nCan't open wave output: %s\n",snd_strerror(err));
			free(buffer);
			return 0;
		}
		snd_pcm_set_params(ahw.pcmhandle,SND_PCM_FORMAT_S16_LE,SND_PCM_ACCESS_RW_INTERLEAVED,2,44100,1,1000);
		if(snd_pcm_writei(ahw.pcmhandle,buffer,2*length*ahw.rate)<0)snd_pcm_prepare(ahw.pcmhandle);
		snd_pcm_close(ahw.pcmhandle);
		free(buffer);
		curtime=clock();
		while(clock()-curtime<length/2.0);
		curpage=&questionpage;
		questionpage.enable();
		return 1;
	}
	void posresponse()
	{
		questionpage.buttons[0].enabled=false;
		if(lastresponse==0)
		{
			cursubtrial++;
			tone.a-=.1;
		}
		else
		{
			cursubtrial=0;
			tone.a-=.4;
		}
		lastresponse=1;
		if(cursubtrial<3)
		{
			curpage=&instructpage;
			instructpage.enable();
		}
		else
		{
			results.push_back((2.0*tone.a+.1)/2.0);
			if(curtrial<trialcount-1)runtrial(++curtrial);
			else
			{
				curpage=&resultspage;
				resultspage.enable();
			}
		}
	}
	void negresponse()
	{
		questionpage.buttons[1].enabled=false;
		if(lastresponse==1)
		{
			cursubtrial++;
			tone.a+=.1;
		}
		else
		{
			cursubtrial=0;
			tone.a+=.4;
		}
		lastresponse=0;
		if(cursubtrial<3)
		{
			curpage=&instructpage;
			instructpage.enable();
		}
		else
		{
			results.push_back((2.0*tone.a-.1)/2.0);
			if(curtrial<trialcount-1)runtrial(++curtrial);
			else
			{
				curpage=&resultspage;
				resultspage.enable();
			}
		}
	}
	void start()
	{
		curtrial=0;
		runtrial(0);
	}
};
ptcexptype experiment;

class trexptype
{
	public:
	short type;//1 for silence gap in continuous tone,2 for silence gap between two pulsed tones
};
			

void resize (int w, int h)
{
	winw=w; 
	winh=h;
	gluOrtho2D(0,winw,winh,0);
	glViewport(0,0,winw,winh);
}

void newexpfunc()
{
	newtypepage.enable();
	homepage.buttons[0].enabled=false;
	curpage=&newtypepage;
}

void setptc()
{
	experiment.startingamp=newptcpage.boxes[12].val;
	experiment.tone.set(newptcpage.boxes[12].val,newptcpage.boxes[0].val,0);
	experiment.n1.set(newptcpage.checkboxes[0].enabled*newptcpage.boxes[6].val,newptcpage.boxes[2].val,newptcpage.boxes[1].val,newptcpage.boxes[5].val);
	experiment.n2.set(newptcpage.checkboxes[1].enabled*newptcpage.boxes[7].val,newptcpage.boxes[3].val,newptcpage.boxes[4].val,newptcpage.boxes[5].val);
	experiment.changefact=newptcpage.boxes[8].val;
	experiment.changetype=newptcpage.ddsels[1].val;
	switch(newptcpage.ddsels[0].val)
	{
		case 0:
			experiment.indvar=&experiment.tone.f;
			break;
		case 1:
			experiment.indvar=&experiment.n1.frange[0];
			break;
		case 2:
			experiment.indvar=&experiment.n1.frange[1];
			break;
		case 3:
			experiment.indvar=&experiment.n2.frange[0];
			break;
		case 4:
			experiment.indvar=&experiment.n2.frange[1];
			break;
	}
	experiment.trialcount=newptcpage.boxes[9].val;
	experiment.length=newptcpage.boxes[10].val;
	experiment.gain=newptcpage.boxes[11].val*100;
}

void newptcfunc()
{
	newexppage.buttons[0].enabled=false;
	newptcpage.enable();
	curpage=&newptcpage;
	GLint bordersize=depthbox(50,20,600,370,.8,.8,.8,.5,-1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(50+bordersize,330+bordersize,550-2*bordersize,350-2*bordersize);
	setptc();
	
	if(newptcpage.checkboxes[0].enabled&&newptcpage.checkboxes[1].enabled)
		gluOrtho2D(experiment.n1.frange[1]-10,experiment.n2.frange[1]+10,0,1);
	else if(newptcpage.checkboxes[0].enabled)
		gluOrtho2D(experiment.n1.frange[1]-10,experiment.tone.f+10,0,1);
	else if(newptcpage.checkboxes[1].enabled)
		gluOrtho2D(experiment.tone.f-10,experiment.n2.frange[1]+10,0,1);
	
	glLineWidth(3);
	glColor3f(0,1,0);
	glBegin(GL_LINES);
		glVertex2f(experiment.tone.f,0);
		glVertex2f(experiment.tone.f,1);
	glEnd();
	glLineWidth(1);
	int i;
	if(newptcpage.checkboxes[0].enabled)
	{
		glColor3f(.3,.3,.3);
		glBegin(GL_LINES);
		for(i=0;i<(experiment.n1.frange[0]-experiment.n1.frange[1])/experiment.n1.ffreq;i++)
		{
			glVertex2f(experiment.n1.frange[0]-i*experiment.n1.ffreq,0);
			glVertex2f(experiment.n1.frange[0]-i*experiment.n1.ffreq,experiment.n1.a);
		}
		glEnd();
	}
	if(newptcpage.checkboxes[1].enabled)
	{
		glColor3f(.3,.3,.3);
		glBegin(GL_LINES);
		for(i=0;i<(experiment.n2.frange[1]-experiment.n2.frange[0])/experiment.n2.ffreq;i++)
		{
			glVertex2f(experiment.n2.frange[0]+i*experiment.n2.ffreq,0);
			glVertex2f(experiment.n2.frange[0]+i*experiment.n2.ffreq,experiment.n2.a);
		}
		glEnd();
	}
}

void startexp()
{	
	setptc();
	experiment.start();
}
void instructfunc()
{
	char temp[10];
	string str="Trial ";
	sprintf(temp,"%d",experiment.curtrial+1);
	str+=temp;
	str+=" of ";
	sprintf(temp,"%d",experiment.trialcount);
	str+=temp;
	dispstring(50,50,str);
	drawparagraph(100,600,100,400,"When you hit the START button below, the trial will begin.  You will here noise which serves to mask a beep.  After the noise ends, you will be asked whether or not you heard the beep.  The pitch of the beep will not change between trials.",0,0,0);
}
void startsound()
{
	curpage=&soundpage;
	soundpage.enable();
}
void soundfunc()
{
	if(experiment.runsubtrial()==0);
}
void questionfunc()
{
	dispstring(50,100,"Did you hear the beep?");
}
void dispresults()
{
	glColor3f(0,0,.5);
	dispstring(50,50,"Results:");
	dispstring(50,100,newptcpage.ddsels[0].options[newptcpage.ddsels[0].val]);
	dispstring(200,100,"Threshold");
	int i;
	glColor3f(0,0,0);
	for(i=0;i<experiment.results.size();i++)
	{
		char temp[64];
		sprintf(temp,"%1.3f",experiment.indvarvals[i]);
		dispstring(50,130+i*30,temp);
		sprintf(temp,"%1.3f",experiment.results[i]);
		dispstring(200,130+i*30,temp);
	}
}
void posresponse()
{
	experiment.posresponse();
}
void negresponse()
{
	experiment.negresponse();
}
void playpreview()
{
	experiment.runsubtrial();
	newptcpage.enable();
	curpage=&newptcpage;
	newptcpage.buttons[0].enabled=false;
}
	
	

void init()
{
	glClearColor(.4,.4,.4,1);
	glShadeModel(GL_SMOOTH);
	glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	winw=920;
	winh=700;
	
	/*char tempchar[64]="hwconfig.txt";
	if(!dlhwinfo(tempchar,&ahw))
	{
		if(!scanalldev(&ahw,1,1))
			cout<<"\n\nIncompatible sound hardware. Program will not run properly.";
		else set_audio_hardware(&ahw,44100,2);
	}
	else 
	{
		set_audio_hardware(&ahw,44100,2);
		ahw.setname(0,0,0);
	}*/
	set_audio_hardware(&ahw,44100,2);
	float c;
	int err;
	if((err=snd_pcm_open(&ahw.pcmhandle,"hw:0,0",SND_PCM_STREAM_PLAYBACK,0))<0)
	{
		printf("\nCan't open wave output: %s\n",snd_strerror(err));
	}
	else
	{
		snd_pcm_set_params(ahw.pcmhandle,SND_PCM_FORMAT_S16_LE,SND_PCM_ACCESS_RW_INTERLEAVED,2,44100,1,1000);
		short *buffer;
		buffer=(short*)malloc(4*10000);
		for(c=0;c<10000;c++)
		{
			buffer[2*(int)c]=(short)(32768*sin(500*c*TWOPI/44100.0));
			buffer[2*(int)c+1]=(short)(32768*sin(500*c*TWOPI/44100.0));
		}
		if(snd_pcm_writei(ahw.pcmhandle,buffer,20000)<0)snd_pcm_prepare(ahw.pcmhandle);
		snd_pcm_close(ahw.pcmhandle);
		free(buffer);
	}
	vector<string> temp;
	
	curpage=&homepage;
	homepage.setcolor(.6,.6,.6);
	homepage.enable();
	homepage.newbut(100,250,200,220,"New Experiment","");
	homepage.buttons.back().setfunc(newexpfunc);
	homepage.newbut(homepage.buttons[0].x,homepage.buttons[0].xend,homepage.buttons[0].yend+10,2*homepage.buttons[0].yend-homepage.buttons[0].y+10,"Load Experiment","");
	homepage.newbut(homepage.buttons[1].x,homepage.buttons[1].xend,homepage.buttons[1].yend+10,2*homepage.buttons[1].yend-homepage.buttons[1].y+10,"Exit Program","");
	
	newtypepage.setcolor(.6,.6,.6);
	newtypepage.newbut(100,300,100,120,"Experiment Type",0,-30,"Auditory Filter Shape","");
	newtypepage.buttons.back().setfunc(newptcfunc);
	newtypepage.newbut(100,300,130,150,"Temporal Resolution","");
	
	newptcpage.setcolor(.6,.6,.6);
	newptcpage.newbox(200,400,20,60,20,1,10,30000,1,500,"Tone Frequency(Hz)","");
	newptcpage.newbox(200,460,20,60,20,1,10,30000,1,300,"L.Noise Start(Hz)","");
	newptcpage.newbox(200,490,20,60,20,1,10,30000,1,490,"L.Noise End(Hz)","");
	newptcpage.newbox(200,550,20,60,20,1,10,30000,1,510,"R.Noise Start(Hz)","");
	newptcpage.newbox(200,580,20,60,20,1,10,30000,1,700,"R.Noise End(Hz)","");
	newptcpage.newbox(200,640,20,60,20,.1,.01,10,1,2,"Noise fq. Spacing(Hz)","");
	newptcpage.newbox(200,520,20,60,20,.1,.01,1,1,.5,"L.Noise Amp.","");
	newptcpage.newbox(200,610,20,60,20,.1,.01,1,1,.5,"R.Noise Amp.","");
	newptcpage.newcheckbox(390,480,"L. Noise",1);
	newptcpage.newcheckbox(390,570,"R. Noise",1);
	temp.clear();
	temp.push_back("Tone Freq.");
	temp.push_back("L.Noise Start");
	temp.push_back("L.Noise End");
	temp.push_back("R.Noise Start");
	temp.push_back("R.Noise End");
	newptcpage.newddsel(600,750,430,450,"Ind.Var.",temp,"");
	temp.clear();
	temp.push_back("Additive");
	temp.push_back("Multiplicative");
	temp.push_back("Exponential");
	newptcpage.newddsel(600,750,460,480,"Change Type",temp,"");
	newptcpage.newbox(600,490,20,60,20,1,-10,10,1,1,"Change","");
	newptcpage.newbox(600,520,20,40,20,1,1,1000,0,10,"Trial Count","");
	newptcpage.newbox(600,550,20,40,20,.5,1.5,30,1,2,"Sound Length(s)","");
	newptcpage.newbox(600,580,20,60,20,1,1,30,0,10,"Gain","");
	newptcpage.newbox(200,430,20,60,20,1,1,20,1,10,"Tone Amp.","");
	newptcpage.newbut(710,790,580,600,"Preview","");
	newptcpage.buttons.back().setfunc(playpreview);
	newptcpage.setfunc(newptcfunc);
	newptcpage.newbut(750,800,660,680,"Next","");
	newptcpage.buttons.back().setfunc(startexp);
	
	instructpage.setcolor(.6,.6,.6);
	instructpage.newbut(700,800,600,630,"Start","");
	instructpage.buttons.back().setfunc(startsound);
	instructpage.setfunc(instructfunc);
	
	soundpage.setcolor(.6,.6,.6);
	soundpage.setfunc(soundfunc);
	
	questionpage.setcolor(.6,.6,.6);
	questionpage.newbut(300,400,300,340,"Yes","");
	questionpage.buttons.back().setfunc(posresponse);
	questionpage.newbut(300,400,400,440,"No","");
	questionpage.buttons.back().setfunc(negresponse);
	questionpage.setfunc(questionfunc);
	
	resultspage.setcolor(.6,.6,.6);
	resultspage.newbut(750,800,660,680,"Next","");
	resultspage.buttons.back().setfunc(newexpfunc);
	resultspage.setfunc(dispresults);
}

void mouseaction(int button,int state,int x,int y)
{
	if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN)curpage->click(x,y);
}

void mousemotion(int x,int y)
{
	mouseloc[0]=x;
	mouseloc[1]=y;
}

void controls(unsigned char key,int x,int y)
{
	curpage->controls(key,x,y);
}
void specialcontrols(int key,int x,int y)
{
	curpage->specialcontrols(key,x,y);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0,0,winw,winh);
	gluOrtho2D(0,winw,winh,0);
	
	curpage->disp();
	glFlush();
}

int main(int argc,char **argv)
{
	glutInit(&argc,argv);
	init();
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(winw,winh);
	glutInitWindowPosition(100,10);
	glutCreateWindow("PTC Measurement App 0.1.01");

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mouseaction);
	glutMotionFunc(mousemotion);
	glutPassiveMotionFunc(mousemotion);
	glutKeyboardFunc(controls);
	glutSpecialFunc(specialcontrols);
	glutMainLoop();
	return 1;
}
