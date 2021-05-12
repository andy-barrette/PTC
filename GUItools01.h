#include<math.h>
#include<cstring>
#include<cstdlib>
#include<cstdio>
#include<vector>
#include<time.h>
#include<GL/glut.h>
using namespace std;

#ifndef STD_FONT
#define STD_FONT GLUT_BITMAP_HELVETICA_18
#endif

#ifndef STD_FONT2
#define STD_FONT2 GLUT_BITMAP_HELVETICA_18
#endif

#ifndef STD_FONT2_HEIGHT
#define STD_FONT2_HEIGHT 12
#endif
#ifndef STD_FONT1_HEIGHT
#define STD_FONT1_HEIGHT 12
#endif

short helpmode=0;
int toplist;
GLint winw,winh,mouseloc[2],textoffset[]={4,4};

void setarray3(GLfloat *vect,GLfloat r,GLfloat g,GLfloat b)
{
	vect[0]=r;
	vect[1]=g;
	vect[2]=b;
}

bool checkrange(float num,float lower,float upper)
{
	if(num<=upper&&lower<=num)return true;
	else return false;
}
void empty(){}

void dispstring(GLint x,GLint y,string str)
{
	glRasterPos2i(x,y);
	for(int i=0;i<str.size();i++)
		glutBitmapCharacter(STD_FONT,str[i]);
}

GLint depthbox(GLint x,GLint y,GLint xright,GLint ylower,GLfloat r,GLfloat g,GLfloat b,GLfloat contrast,short depth)//contrast must be between 0 and 1
{
	short bordersize,xsize=((xright-x)/100)+1,ysize=((ylower-y)/100)+1;
	if(xsize>4)xsize=4;
	if(ysize>4)ysize=4;
	if(xsize>ysize)bordersize=ysize;
	else bordersize=xsize;
	
	glColor3f(r,g,b);
	glRecti(x,y,xright,ylower);
	
	GLint i;
	for(i=0;i<bordersize;i++)
	{
		if(depth==1)glColor3f(.2+.1*i,.2+.1*i,.2+.1*i);
		else if(depth==-1)glColor3f(.9-.1*i,.9-.1*i,.9-.1*i);
		else glColor3f(0,0,0);
		glBegin(GL_LINE_STRIP);
			glVertex2i(xright-i,y+i);
			glVertex2i(xright-i,ylower-i);
			glVertex2i(x+i,ylower-i);
		glEnd();
		
		if(depth==1)glColor3f(.9-.1*i,.9-.1*i,.9-.1*i);
		else if(depth==-1)glColor3f(.2+.1*i,.2+.1*i,.2+.1*i);
		else glColor3f(0,0,0);
		glBegin(GL_LINE_STRIP);
			glVertex2i(x+i,ylower-i);
			glVertex2i(x+i,y+i);
			glVertex2i(xright-i,y+i);
		glEnd();
	}
	return bordersize;
}

short wordlen(string text,short curloc)
{
	short i,len=glutBitmapWidth(STD_FONT2,text[curloc]);
	for(i=curloc+1;i<text.size();i++)
	{
		if(text[i]!=' '&&text[i]!='\n'&&text[i]!='\0')
			len+=glutBitmapWidth(STD_FONT2,text[i]);
		else break;
	}
	return len;
}
short textlen(string text)
{
	short i,len=glutBitmapWidth(STD_FONT2,text[0]);
	for(i=1;i<text.size();i++)
	{
		len+=glutBitmapWidth(STD_FONT2,text[i]);
	}
	return len;
}

void drawparagraph(GLint x,GLint xend,GLint y,GLint yend,string text,GLfloat r,GLfloat g,GLfloat b)
{
	short int i;
	if(x>xend)
	{
		i=x;
		x=xend;
		xend=i;
	}
	if(y>yend)
	{
		i=y;
		y=yend;
		yend=i;
	}
	short curloc[2]={x,y};

	glColor3f(r,g,b);
	glRasterPos2i(x,y);
	for(i=0;i<text.size();i++)
	{
		if(text[i]!='\n'&&(text[i]==' '||curloc[0]+wordlen(text,i)<xend||wordlen(text,i)>=xend-x))
		{
			curloc[0]+=glutBitmapWidth(STD_FONT2,text[i]);
			glutBitmapCharacter(STD_FONT2,text[i]);
		}
		else
		{
			curloc[1]+=(STD_FONT2_HEIGHT*1.5);
			if(curloc[1]<yend)
			{
				glRasterPos2i(x,curloc[1]);
				curloc[0]=x;
			}
			else break;
			glutBitmapCharacter(STD_FONT2,text[i]);
		}
	}
}

class helptype
{
	GLint x,y;
	string text;
	
	public:
	void set(GLint a,GLint b,string h)
	{
		helpmode=2;
		x=a;
		y=b;
		text=h;
	}
	void disp()
	{
		short direc[2],i;
		GLint h=0;
	
		if(x<winw/2)direc[0]=1;
		else direc[0]=-1;
		if(y<winh/2)direc[1]=1;
		else direc[1]=-1;
	
	
		for(i=0;i<text.size();i++)
			h+=glutBitmapWidth(GLUT_BITMAP_HELVETICA_12,text[i]);
		h=STD_FONT2_HEIGHT*(h/260)+1;
	
		depthbox(x,y,x+direc[0]*300,y+direc[1]*(35+h),1,1,.7,.5,1);
		drawparagraph(x+direc[0]*20,x+direc[0]*280,y+direc[1]*20,y+direc[1]*(20+h),text,0,0,0);
		helpmode=1;
	}
}helpvars;

class scrollbartype
{
	GLint height,width;
	
	public:
	GLint pan;
	bool selected;
	void set(GLint a,GLint b)
	{
		width=a;
		height=b;
		pan=0;
	}
	bool inrange(GLint locx,GLint locy)
	{
		if(0<locx&&locx<20&&0<locy&&locy<winh)return true;
		else return false;
	}		
	void disp()
	{
		depthbox(0,0,20,winh,.9,.9,.9,.5,-1);
		depthbox(0,pan-height/2,20,pan+height/2,.7,.7,.7,.5,1);
	}	
	void move(GLint change)
	{
		short free;
		if(change>0)
		{
			if(pan<winh-height/2)pan+=change;
		}
		else if(change<0)
			if(pan>height)pan-=change;
	}
}scrollbar;

class toggletype
{
	vector<string> text;
	string help,othertext;
	GLfloat color[3],hovercolor[3],clickcolor[3];
	bool direc;//0==horizontal,1==vertical
	
	public:
	GLint x,xend,y,yend,type,otdisp[2];
	short enabled;
	void set(GLint a,GLint b,GLint c,GLint d,bool dir,vector<string> str,string h)
	{
		x=a;
		xend=b;
		y=c;
		yend=d;
		direc=dir;
		text=str;
		help=h;
		
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
		enabled=1;
	}
	void set(GLint a,GLint b,GLint c,GLint d,string ot,GLint otx,GLint oty,bool dir,vector<string> str,string h)
	{
		x=a;
		xend=b;
		y=c;
		yend=d;
		direc=dir;
		text=str;
		help=h;
		
		othertext=ot;
		otdisp[0]=otx;
		otdisp[1]=oty;
		ot=true;
		
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
		enabled=1;
	}
	void shift(GLint dx,GLint dy)
	{
		x+=dx;
		xend+=dx;
		y+=dy;
		yend+=dy;
	}
	short inrange(GLint locx,GLint locy)
	{
		int i;
		if(direc==false)
		{
			for(i=0;i<text.size();i++)
				if(x+(xend-x)*i<locx&&locx<xend+(xend-x)*i&&y<locy&&locy<yend)return i+1;
		}
		else
		{
			for(i=0;i<text.size();i++)
				if(x<locx&&locx<xend&&y+(yend-y)*i<locy&&locy<yend+(yend-y)*i)return i+1;
		}
		return 0;
	}
	void disp()
	{
		int i;
		
		if(direc)
			for(i=0;i<text.size();i++)
			{
				if(i+1==enabled)
					depthbox(x,y+(yend-y)*i,xend,yend+(yend-y)*i,clickcolor[0],clickcolor[1],clickcolor[2],.5,-1);	
				else
				{
					if(inrange(mouseloc[0],mouseloc[1]))
						depthbox(x,y+(yend-y)*i,xend,yend+(yend-y)*i,hovercolor[0],hovercolor[1],hovercolor[2],.5,1);
					else depthbox(x,y+(yend-y)*i,xend,yend+(yend-y)*i,color[0],color[1],color[2],.5,1);
				}
			}
		else
			for(i=0;i<text.size();i++)
			{
				if(i+1==enabled)
					depthbox(x+(xend-x)*i,y,xend+(xend-x)*i,yend,clickcolor[0],clickcolor[1],clickcolor[2],.5,-1);	
				else
				{
					if(inrange(mouseloc[0],mouseloc[1]))
						depthbox(x+(xend-x)*i,y,xend+(xend-x)*i,yend,hovercolor[0],hovercolor[1],hovercolor[2],.5,1);
					else depthbox(x+(xend-x)*i,y,xend+(xend-x)*i,yend,color[0],color[1],color[2],.5,1);
				}
			}
		
		
		glColor3f(0,0,0);
		
		int n;
		for(n=0;n<text.size();n++)
		{
			if(direc)glRasterPos2i(x+textoffset[0],yend+(yend-y)*n-textoffset[1]);
			else glRasterPos2i(x+(xend-x)*n+textoffset[0],yend-textoffset[1]);
			for(i=0;i<text[n].size();i++)
				glutBitmapCharacter(STD_FONT,text[n][i]);
		}
		if(helpmode)
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);
	}
	short click(GLint locx,GLint locy)
	{
		int result=inrange(locx,locy);
		if(result>0)enabled=result;
		return result;
	}
};

class buttontype
{
	string text,othertext,help;
	GLfloat color[3],hovercolor[3],clickcolor[3];
	void (*func)();
	int *fptr;
	GLint otdisp[2];
	bool ot;
	
	public:
	GLint x,xend,y,yend,type;
	bool enabled;
	void set(GLint a,GLint b,GLint c,GLint d,string str,string h)
	{
		x=a;
		xend=b;
		y=c;
		yend=d;
		ot=false;
		text=str;
		help=h;
		func=&empty;
		
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
		enabled=false;
	}
	void set(GLint a,GLint b,GLint c,GLint d,string ot,GLint otx,GLint oty,string str,string h)
	{
		x=a;
		xend=b;
		y=c;
		yend=d;
		othertext=ot;
		otdisp[0]=otx;
		otdisp[1]=oty;
		ot=true;
		text=str;
		help=h;
		func=&empty;
		
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
		enabled=false;
	}
	void setfunc(void (*f)())
	{
		func=f;
	}
	void shift(GLint dx,GLint dy)
	{
		x+=dx;
		xend+=dx;
		y+=dy;
		yend+=dy;
	}
	void invert()
	{
		enabled=!enabled;
	}
	bool inrange(GLint locx,GLint locy)
	{
		if(x<locx&&locx<xend&&y<locy&&locy<yend) return true;
		else return false;
	}
	void disp()
	{
		int i;
		
		if(enabled)
			depthbox(x,y,xend,yend,clickcolor[0],clickcolor[1],clickcolor[2],.5,-1);
		else
		{
			if(inrange(mouseloc[0],mouseloc[1]))
				depthbox(x,y,xend,yend,hovercolor[0],hovercolor[1],hovercolor[2],.5,1);
			else depthbox(x,y,xend,yend,color[0],color[1],color[2],.5,1);
		}
		
		
		glColor3f(0,0,0);
		if(ot)
			dispstring(x+otdisp[0]+textoffset[0],yend+otdisp[1]-textoffset[1],othertext);
		dispstring(x+((xend-x)-textlen(text))/2,y+((yend-y)+STD_FONT1_HEIGHT)/2,text);
		if(helpmode)
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);
	}
	
	bool click(GLint locx,GLint locy)
	{
		if(inrange(locx,locy))
		{
			invert();
			func();
			return 1;
		}
		else return 0;
	}
};

class fieldtype
{
	short cursorplace,fieldtype;//type1==numeric, type2==alphanumeric
	GLint cursorloc;
	string help,othertext;
	GLint otdisp[2];
	bool ot;
	
	public:
	string text;
	GLint x,xend,y,yend;
	bool enabled;
	void set(GLint a,GLint b,GLint c,GLint d,short t,string h)
	{
		x=a;
		y=b;
		xend=c;
		yend=d;
		fieldtype=t;
		enabled=false;
		help=h;
	}
	void set(GLint a,GLint b,GLint c,GLint d,string ot,GLint otx,GLint oty,short t,string h)
	{
		x=a;
		y=b;
		xend=c;
		yend=d;
		othertext=ot;
		otdisp[0]=otx;
		otdisp[1]=oty;
		ot=true;
		fieldtype=t;
		enabled=false;
		help=h;
	}
	void shift(GLint dx,GLint dy)
	{
		x+=dx;
		xend+=dx;
		y+=dy;
		yend+=dy;
	}
	void specialtype(int key)
	{
		if(key==GLUT_KEY_LEFT&&cursorplace>0)
		{
			cursorplace--;
			cursorloc-=glutBitmapWidth(STD_FONT,text[cursorplace]);
		}
		else if(key==GLUT_KEY_RIGHT&&cursorplace<text.size())
		{
			cursorloc+=glutBitmapWidth(STD_FONT,text[cursorplace]);
			cursorplace++;
		}
	}
	short textsize()
	{
		int i,size=0;
		for(i=0;i<text.size();i++)
			size+=glutBitmapWidth(STD_FONT,text[i]);
		return size;
	}
	bool inrange(GLint locx,GLint locy)
	{
		if(x<locx&&locx<xend&&y<locy&&locy<yend) return true;
		else return false;
	}
	void disp()
	{
		depthbox(x,y,xend,yend,1,1,1,.5,-1);
		glColor3f(0,0,0);
		if(ot)
			dispstring(x+otdisp[0]+textoffset[0],yend+otdisp[1]-textoffset[1],othertext);
		dispstring(x+textoffset[0],yend-textoffset[1],text);
		if(helpmode)
		{
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);
		}
		else if(enabled&&clock()%800000<600000)
		{
			glBegin(GL_LINES);
				glVertex2f(cursorloc+x,y+2);
				glVertex2f(cursorloc+x,yend-2);
			glEnd();
		}
	}
	
	int click(int xloc,int yloc)
	{
		int n,m;
		double placestart,placeend;
		if(inrange(xloc,yloc))
		{
			xloc-=x;
			cursorplace=text.size();
			cursorloc=0;
			placestart=0;
			placeend=textoffset[0]+(glutBitmapWidth(STD_FONT,text[0])/2.0);
	
			for(m=0;m<cursorplace;m++,placestart=placeend,placeend+=(glutBitmapWidth(STD_FONT,text[m-1])/2.0)+(glutBitmapWidth(STD_FONT,text[m])/2.0))
				if(placestart<xloc&&xloc<=placeend)
				{
					cursorplace=m;
					cursorloc=placeend-glutBitmapWidth(STD_FONT,text[m])/2;
					break;
				}
			if(cursorloc==0)
				cursorloc=placeend;
			enabled=true;
			return n;
		}
		else
		{
			enabled=false;
			return -1;
		}
	}

	bool type(char a)
	{
		if(enabled)
		{
			if(fieldtype==1)
			{
				if(checkrange(a,48,57)||a==45||a==46)
				{
					if(textsize()+glutBitmapWidth(STD_FONT,a)<xend-x-2*textoffset[0])
					{
						text.insert(cursorplace,1,a);
						cursorplace++;
						cursorloc+=glutBitmapWidth(STD_FONT,a);
					}
					return true;
				}
			}
			else if(checkrange(a,48,57)||checkrange(a,65,90)||checkrange(a,97,122))
			{
				if(textsize()+glutBitmapWidth(STD_FONT,a)<xend-x-2*textoffset[0]&&(((int)a>=48&&(int)a<=122)||a==' '))
				{
					text.insert(cursorplace,1,a);
					cursorplace++;
					cursorloc+=glutBitmapWidth(STD_FONT,a);
				}
				return true;
			}
			if(a==13)
			{
				enabled=false;
				return false;
			}
			if(cursorplace>0&&a==8)
			{
				cursorloc-=glutBitmapWidth(STD_FONT,text[cursorplace-1]);
				text.erase(cursorplace-1,1);
				cursorplace--;
				return true;
			}
			
		}
		else return true;
	}
};

class incrementboxtype
{
	GLint arrowwidth,boxwidth,height;
	float min,max,increment;
	string text,help;
	void (*func)();
	
	public:
	GLint textx,x,xend,y,yend;
	short numtype;
	float val;
	fieldtype field;
	void recalcx()
	{
		int n;
		char tempchar[10];
		textx=x;
		for(n=0;n<text.size();n++)
			textx-=glutBitmapWidth(STD_FONT,text[n]);
		textx-=5;
		xend=x+arrowwidth*2+4+boxwidth;
		yend=y+height;
		
		field.set(x+arrowwidth+2,y,x+arrowwidth+boxwidth,y+height,1,help);
		if(numtype)
			sprintf(tempchar,"%1.1f",val);
		else
			sprintf(tempchar,"%d",(int)val);
		field.text=tempchar;
	}
	void set(GLint a,GLint b,GLint c,GLint d,GLint e,float f,float mi,float ma,short nt,float startval,string g,string h)
	{
		x=a;
		y=b;
		arrowwidth=c;
		boxwidth=d;
		height=e;
		increment=f;
		min=mi;
		max=ma;
		numtype=nt;
		val=startval;
		text=g;
		help=h;
		func=&empty;
		
		recalcx();
	}
	void setfunc(void (*f)())
	{
		func=f;
	}
	void shift(GLint dx,GLint dy)
	{
		x+=dx;
		xend+=dx;
		textx+=dx;
		y+=dy;
		yend+=dy;
		field.shift(dx,dy);
	}
	bool inrange(GLint locx,GLint locy)
	{
		if(x<locx&&locx<xend&&y<locy&&locy<yend&&!field.inrange(locx,locy))return true;
		else return false;
	}
	void disp()
	{
		char tempchar[10];
		
		glColor3f(0,0,0);
		dispstring(textx,y+height,text);
		
		depthbox(x,y,x+arrowwidth,y+height,.7,.7,.7,.5,1);
		depthbox(x+arrowwidth+4+boxwidth,y,x+2*arrowwidth+4+boxwidth,y+height,.7,.7,.7,.5,1);
		
		field.disp();
		
		glColor3f(0,0,0);
		glBegin(GL_POLYGON);
			glVertex2i(x+arrowwidth-3,y+3);
			glVertex2i(x+3,y+(height/2));
			glVertex2i(x+arrowwidth-3,y+height-3);
		glEnd();
		glBegin(GL_POLYGON);
			glVertex2i(x+arrowwidth+4+boxwidth+3,y+3);
			glVertex2i(x+2*arrowwidth+4+boxwidth-3,y+(height/2));
			glVertex2i(x+arrowwidth+4+boxwidth+3,y+height-3);
		glEnd();
		
		if(helpmode)
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);
		func();			
	}
	void checksize()
	{
		float tempval=val;
		if(val>max)val=max;
		else if(val<min)val=min;
	}
	bool type(char key)
	{
		char tempchar[10];
		if(field.type(key)==false)
		{
			for(int i=0;i<field.text.size(),i<10;i++)
				tempchar[i]=field.text[i];
			if(atof(tempchar)==val)return false;
			else
			{
				val=atof(tempchar);
				checksize();
				return true;
			}
		}
		else return false;
	}
	short click(GLint locx,GLint locy)
	{
		char tempchar[10];
		if(field.enabled)
		{
			if(field.click(locx,locy)==-1)
			{
				for(int i=0;i<field.text.size(),i<10;i++)
					tempchar[i]=field.text[i];
				if(atof(tempchar)==val)return 0;
				else
				{
					val=atof(tempchar);
					checksize();
					return 1;
				}
			}
			else return 0;
		}
		else if(x<locx&&locx<x+arrowwidth&&y<locy&&locy<y+height)
		{
			val-=increment;
			checksize();
			
			if(numtype)
				sprintf(tempchar,"%1.1f",val);
			else
				sprintf(tempchar,"%d",(int)val);
			field.text=tempchar;
			return -1;
		}
		else if(x+boxwidth+arrowwidth+4<locx&&locx<x+2*arrowwidth+boxwidth+4&&y<locy&&locy<y+height)
		{
			val+=increment;
			checksize();
			
			if(numtype)
				sprintf(tempchar,"%1.1f",val);
			else
				sprintf(tempchar,"%d",(int)val);
			field.text=tempchar;
			return 1;
		}
		else
		{
			field.click(locx,locy);
			return 0;
		}
	}
};

class checkboxtype
{
	GLint x,y,textx;//(x,y) is lower-left corner
	string text;
	
	public:
	bool enabled;
	void recalcx()
	{
		int n;
		char tempchar[10];
		textx=x;
		for(n=0;n<text.size();n++)
			textx-=glutBitmapWidth(STD_FONT,text[n]);
		textx-=5;
	}
	void set(GLint tx,GLint ty,string str,bool init)
	{
		x=tx;
		y=ty;
		text=str;
		enabled=init;
		recalcx();
	}
	void shift(GLint dx,GLint dy)
	{
		x+=dx;
		textx+=dx;
		y+=dy;
	}	
	void disp()
	{
		depthbox(x,y-textoffset[1],x+16,y-16-textoffset[1],.8,.8,.8,.5,-1);
		glColor3f(0,0,0);
		dispstring(textx,y-textoffset[1],text);
		if(enabled)
		{
			glLineWidth(3);
			glBegin(GL_LINE_STRIP);
				glVertex2i(x+1,y-8-textoffset[1]);
				glVertex2i(x+8,y-1-textoffset[1]);
				glVertex2i(x+15,y-15-textoffset[1]);
			glEnd();
			glLineWidth(1);
		}
	}
	bool click(GLint locx,GLint locy)
	{
		if(locx>x&&locx<x+20&&locy<y&&locy>y-20)
		{
			enabled=!enabled;
			return true;
		}
		else return false;
	}
};
class ddseltype
{
	GLint x,xend,y,yend,textx;
	string text,help;
	bool enabled;
	GLfloat color[3],hovercolor[3],clickcolor[3];
	
	void recalcx()
	{
		int n;
		char tempchar[10];
		textx=x;
		for(n=0;n<text.size();n++)
			textx-=glutBitmapWidth(STD_FONT,text[n]);
		textx-=5;
	}
	short inrange(GLint locx,GLint locy)
	{
		int i;
		for(i=0;i<options.size();i++)
			if(x<locx&&locx<xend&&yend+(yend-y)*i<locy&&locy<2*yend-y+(yend-y)*i)return i;
		return -1;
	}
	public:
	vector<string> options;
	short val;
	void set(GLint tx,GLint txend,GLint ty,GLint tyend,string ttext,vector<string> opt,string thelp)
	{
		x=tx;
		xend=txend;
		y=ty;
		yend=tyend;
		text=ttext;
		options=opt;
		help=thelp;
		setarray3(color,1,1,1);
		setarray3(hovercolor,.5,.5,.7);
		setarray3(clickcolor,.5,.5,.7);
		recalcx();
		val=0;
		enabled=false;
	}
	void shift(GLint dx,GLint dy)
	{
		x+=dx;
		xend+=dx;
		textx+=dx;
		y+=dy;
		yend+=dy;
	}	
	void disp()
	{
		depthbox(x,y,xend,yend,.9,.9,.9,.5,-1);
		glColor3f(0,0,0);
		dispstring(textx,yend-textoffset[1],text);
		dispstring(x+textoffset[0],yend-textoffset[1],options[val]);
		
		depthbox(xend,y,xend+20,yend,.7,.7,.7,.5,1);
		glColor3f(0,0,0);
		glBegin(GL_POLYGON);
			glVertex2i(xend+3,y+3);
			glVertex2i(xend+17,y+3);
			glVertex2i(xend+10,yend-3);
		glEnd();
		if(enabled)
		{			
			int i;
			toplist=glGenLists(1);
			glNewList(toplist,GL_COMPILE);
				depthbox(x,yend,xend,yend+options.size()*(yend-y),1,1,1,.5,-1);
				glColor3f(.5,.5,.5);
				glBegin(GL_LINES);
				for(i=1;i<options.size();i++)
				{
					glVertex2f(x,yend+(yend-y)*i);
					glVertex2f(xend,yend+(yend-y)*i);
				}
				glEnd();
				glColor3f(0,0,0);
				for(i=1;i<=options.size();i++)
				{
					if(inrange(mouseloc[0],mouseloc[1])==i-1)
					{
						glColor3f(.7,.7,.9);
						glRecti(x+1,yend+(i-1)*(yend-y)+2,xend-1,yend+i*(yend-y)-2);
						glColor3f(0,0,0);
					}
					dispstring(x+textoffset[0],yend+i*(yend-y)-textoffset[1],options[i-1]);
				}
			glEndList();
		}
	}
	bool click(GLint locx,GLint locy)
	{
		if(xend<locx&&locx<xend+20&&y<locy&&locy<yend)
		{
			enabled=!enabled;
			return true;
		}
		if(enabled)
		{
			if(inrange(locx,locy)>-1)
			{
				val=inrange(locx,locy);
				enabled=false;
				return true;
			}
			enabled=false;
			return false;
		}
	}
};

class paneltype
{
	std::vector<incrementboxtype*> boxes;
	std::vector<buttontype*> buttons;
	std::vector<fieldtype*> fields;
	std::vector<toggletype*> togs;
	std::vector<checkboxtype*> checkboxes;
	std::vector<ddseltype*> ddsels;
	GLfloat color[3];
	
	public:
	string text;
	buttontype but;
	GLint loc[2][2];
	void set(GLint x,GLint y,GLint xend,GLint yend,string str,GLfloat r,GLfloat g,GLfloat b)
	{
		loc[0][0]=x;
		loc[0][1]=y;
		loc[1][0]=xend;
		loc[1][1]=yend;
		text=str;
		color[0]=r;
		color[1]=g;
		color[2]=b;
		int i;
		GLint textlen=0;
		for(i=0;i<str.size();i++)
			textlen+=glutBitmapWidth(STD_FONT,text[i]);
		but.set(x+30+textlen,x+30+textlen+15,loc[0][1]+5,loc[0][1]+20,"","Show/Hide panel.");
	}
	void shifttools(GLint dx,GLint dy)
	{
		int i;
		for(i=0;i<boxes.size();i++)
			boxes[i]->shift(dx,dy);
		for(i=0;i<buttons.size();i++)
			buttons[i]->shift(dx,dy);
		for(i=0;i<fields.size();i++)
			fields[i]->shift(dx,dy);
		for(i=0;i<togs.size();i++)
			togs[i]->shift(dx,dy);
		for(i=0;i<checkboxes.size();i++)
			checkboxes[i]->shift(dx,dy);
		for(i=0;i<ddsels.size();i++)
			ddsels[i]->shift(dx,dy);
	}
	void shift(GLint dx,GLint dy)
	{
		loc[0][0]+=dx;
		loc[1][0]+=dx;
		loc[0][1]+=dy;
		loc[1][1]+=dy;
		but.shift(dx,dy);
		shifttools(dx,dy);
	}
	void disp()
	{
		if(but.enabled)
		{
			depthbox(loc[0][0],loc[0][1]+30,loc[1][0],loc[1][1],color[0],color[1],color[2],.5,-1);
			
			int i;
			GLint vpvals[4];
			
			for(i=0;i<boxes.size();i++)
				boxes[i]->disp();
			for(i=0;i<buttons.size();i++)
				buttons[i]->disp();
			for(i=0;i<fields.size();i++)
				fields[i]->disp();
			for(i=0;i<togs.size();i++)
				togs[i]->disp();
			for(i=0;i<checkboxes.size();i++)
				checkboxes[i]->disp();
			for(i=0;i<ddsels.size();i++)
				ddsels[i]->disp();
		}
	}
	bool click(GLint x,GLint y)
	{
		if(but.enabled)
		{
			int i;
			for(i=0;i<boxes.size();i++)
				if(boxes[i]->click(x,y))return true;
			for(i=0;i<buttons.size();i++)
				if(buttons[i]->click(x,y))return true;
			for(i=0;i<fields.size();i++)
				if(fields[i]->click(x,y))return true;
			for(i=0;i<togs.size();i++)
				if(togs[i]->click(x,y))return true;
			for(i=0;i<checkboxes.size();i++)
				if(checkboxes[i]->click(x,y))return true;
			for(i=0;i<ddsels.size();i++)
				if(ddsels[i]->click(x,y))return true;
		}
		return false;
	}
	void type(unsigned char key)
	{
		int i;
		for(i=0;i<fields.size();i++)
		{
			if(fields[i]->enabled)fields[i]->type(key);
		}
		for(i=0;i<boxes.size();i++)
		{
			if(boxes[i]->field.enabled)boxes[i]->type(key);
		}
	}
	void specialtype(int key)
	{
		int i;
		for(i=0;i<fields.size();i++)
		{
			if(fields[i]->enabled)fields[i]->specialtype(key);
		}
		for(i=0;i<boxes.size();i++)
		{
			if(boxes[i]->field.enabled)boxes[i]->field.specialtype(key);
		}
	}
	
	void add(incrementboxtype *f)
	{
		boxes.push_back(f);
	}
	void add(buttontype *f)
	{
		buttons.push_back(f);
	}
	void add(fieldtype *f)
	{
		fields.push_back(f);
	}
	void add(toggletype *f)
	{
		togs.push_back(f);
	}
	void add(checkboxtype *f)
	{
		checkboxes.push_back(f);
	}
	void add(ddseltype *f)
	{
		ddsels.push_back(f);
	}
};

class expandmenutype
{
	vector<paneltype*> panels;
	GLint x,y;
	public:
	void set(GLint tx,GLint ty)
	{
		x=tx;
		y=ty;
	}
	void disp()
	{
		int i;
		GLint ycur=0;
		
		for(i=0;i<panels.size();i++)
		{			
			panels[i]->disp();
			
			glColor3f(0,0,0);
			dispstring(panels[i]->loc[0][0]+10,panels[i]->loc[0][1]+20,panels[i]->text);
			panels[i]->but.disp();
			glColor3f(0,0,0);
			glBegin(GL_LINES);
				glVertex2i(panels[i]->but.x+3,panels[i]->but.y+(panels[i]->but.yend-panels[i]->but.y)/2);
				glVertex2i(panels[i]->but.xend-3,panels[i]->but.y+(panels[i]->but.yend-panels[i]->but.y)/2);
				if(panels[i]->but.enabled==0)
				{
					glVertex2i(panels[i]->but.x+(panels[i]->but.xend-panels[i]->but.x)/2,panels[i]->but.y+3);
					glVertex2i(panels[i]->but.x+(panels[i]->but.xend-panels[i]->but.x)/2,panels[i]->but.yend-4);
				}
			glEnd();
			ycur+=panels[i]->but.enabled*panels[i]->loc[1][1]+20;
		}
	}
	bool click(GLint xloc,GLint yloc)
	{
		short i,n;
		GLint ycur=y;
		if(panels.size()>0)
			for(i=0;i<panels.size();i++)
			{
				if(panels[i]->but.click(xloc,yloc))
				{
					if(panels[i]->but.enabled)
					{
						for(n=i+1;n<panels.size();n++)
							panels[n]->shift(0,panels[i]->loc[1][1]-panels[i]->loc[0][1]-20);
					}
					else
					{
						for(n=i+1;n<panels.size();n++)
							panels[n]->shift(0,panels[i]->loc[0][1]-panels[i]->loc[1][1]+20);
					}
					return true;
				}
				else if(panels[i]->but.enabled&&panels[i]->click(xloc,yloc))return true;
			}
		return false;
	}
	void type(char key)
	{
		int i;
		for(i=0;i<panels.size();i++)
			panels[i]->type(key);
	}
	void specialtype(char key)
	{
		int i;
		for(i=0;i<panels.size();i++)
			panels[i]->specialtype(key);
	}
	void add(paneltype *p)
	{
		panels.push_back(p);
		if(panels.size()==1)
			panels[0]->shift(x,y);
		else if(panels.size()>=2)
		{
			panels[panels.size()-1]->shift(x,panels[panels.size()-2]->loc[0][1]+panels[panels.size()-2]->loc[1][1]*panels[panels.size()-2]->but.enabled+20);
		}
	}
};

class pagetype
{
	public:
	std::vector<paneltype> panels;
	std::vector<expandmenutype> expandmenus;
	std::vector<incrementboxtype> boxes;
	std::vector<buttontype> buttons;
	std::vector<fieldtype> fields;
	std::vector<toggletype> togs;
	std::vector<checkboxtype> checkboxes;
	std::vector<ddseltype> ddsels;
	GLfloat color[3];
	void (*func)();
	
	pagetype()
	{
		func=&empty;
	}
	
	void setfunc(void (*f)())
	{
		func=f;
	}
	void setcolor(GLfloat r,GLfloat g,GLfloat b)
	{
		color[0]=r;
		color[1]=g;
		color[2]=b;
	}
	void enable()
	{
		glClearColor(color[0],color[1],color[2],1);
	}
	void disp()
	{
		int i;
		for(i=0;i<panels.size();i++)
			panels[i].disp();
		for(i=0;i<expandmenus.size();i++)
			expandmenus[i].disp();
		for(i=0;i<boxes.size();i++)
			boxes[i].disp();
		for(i=0;i<buttons.size();i++)
			buttons[i].disp();
		for(i=0;i<fields.size();i++)
			fields[i].disp();
		for(i=0;i<togs.size();i++)
			togs[i].disp();
		for(i=0;i<checkboxes.size();i++)
			checkboxes[i].disp();
		for(i=0;i<ddsels.size();i++)
			ddsels[i].disp();
		glCallList(toplist);
		glDeleteLists(toplist,1);
		func();
	}
	void click(GLint x,GLint y)
	{
		int i;
		for(i=0;i<panels.size();i++)
			panels[i].click(x,y);
		for(i=0;i<expandmenus.size();i++)
			expandmenus[i].click(x,y);
		for(i=0;i<boxes.size();i++)
			boxes[i].click(x,y);
		for(i=0;i<buttons.size();i++)
			buttons[i].click(x,y);
		for(i=0;i<fields.size();i++)
			fields[i].click(x,y);
		for(i=0;i<togs.size();i++)
			togs[i].click(x,y);
		for(i=0;i<checkboxes.size();i++)
			checkboxes[i].click(x,y);
		for(i=0;i<ddsels.size();i++)
			ddsels[i].click(x,y);
	}
	void controls(unsigned char key,int x,int y)
	{
		int i;
		for(i=0;i<fields.size();i++)
		{
			if(fields[i].enabled)fields[i].type(key);
		}
		for(i=0;i<boxes.size();i++)
		{
			if(boxes[i].field.enabled)boxes[i].type(key);
		}
		for(i=0;i<expandmenus.size();i++)
		{
			expandmenus[i].type(key);
		}
		for(i=0;i<panels.size();i++)
		{
			panels[i].type(key);
		}
	}
	void specialcontrols(int key,int x,int y)
	{
		int i;
		for(i=0;i<fields.size();i++)
		{
			if(fields[i].enabled)fields[i].specialtype(key);
		}
		for(i=0;i<boxes.size();i++)
		{
			if(boxes[i].field.enabled)boxes[i].field.specialtype(key);
		}
		for(i=0;i<expandmenus.size();i++)
		{
			expandmenus[i].specialtype(key);
		}
		for(i=0;i<panels.size();i++)
		{
			panels[i].specialtype(key);
		}
	}
	void newexpandmenu(GLint tx,GLint ty)
	{
		expandmenutype temp;
		temp.set(tx,ty);
		expandmenus.push_back(temp);
	}
	void newpanel(GLint x,GLint y,GLint xend,GLint yend,string str,GLfloat r,GLfloat g,GLfloat b)
	{
		paneltype temp;
		temp.set(x,y,xend,yend,str,r,g,b);
		panels.push_back(temp);
	}
	void newbox(GLint a,GLint b,GLint c,GLint d,GLint e,float f,float mi,float ma,short nt,float startval,string g,string h)
	{
		incrementboxtype temp;
		temp.set(a,b,c,d,e,f,mi,ma,nt,startval,g,h);
		boxes.push_back(temp);
	}
	void newbut(GLint a,GLint b,GLint c,GLint d,string ot,GLint otx,GLint oty,string str,string h)
	{
		buttontype temp;
		temp.set(a,b,c,d,ot,otx,oty,str,h);
		buttons.push_back(temp);
	}
	void newbut(GLint a,GLint b,GLint c,GLint d,string str,string h)
	{
		buttontype temp;
		temp.set(a,b,c,d,str,h);
		buttons.push_back(temp);
	}
	void newfield(GLint a,GLint b,GLint c,GLint d,short t,string h)
	{
		fieldtype temp;
		temp.set(a,b,c,d,t,h);
		fields.push_back(temp);
	}
	void newfield(GLint a,GLint b,GLint c,GLint d,string ot,GLint otx,GLint oty,short t,string h)
	{
		fieldtype temp;
		temp.set(a,b,c,d,ot,otx,oty,t,h);
		fields.push_back(temp);
	}
	void newtog(GLint a,GLint b,GLint c,GLint d,bool dir,vector<string> str,string h)
	{
		toggletype temp;
		temp.set(a,b,c,d,dir,str,h);
		togs.push_back(temp);
	}
	void newtog(GLint a,GLint b,GLint c,GLint d,string ot,GLint otx,GLint oty,bool dir,vector<string> str,string h)
	{
		toggletype temp;
		temp.set(a,b,c,d,ot,otx,oty,dir,str,h);
		togs.push_back(temp);
	}
	void newcheckbox(GLint tx,GLint ty,string str,bool init)
	{
		checkboxtype temp;
		temp.set(tx,ty,str,init);
		checkboxes.push_back(temp);
	}
	void newddsel(GLint tx,GLint txend,GLint ty,GLint tyend,string ttext,vector<string> opt,string thelp)
	{
		ddseltype temp;
		temp.set(tx,txend,ty,tyend,ttext,opt,thelp);
		ddsels.push_back(temp);
	}
};

class dropdowntype
{
	vector<dropdowntype> option;
	string text;
	GLint x,y,width,height;
	short enabled;
	bool direc;//0=left,1=right
	void (*func)();
	GLfloat color[3],hovercolor[3],clickcolor[3];
	
	short inrange(GLint locx,GLint locy)
	{
		int i;
		for(i=0;i<option.size();i++)
			if(x<locx&&locx<x+width&&y+height*i<locy&&locy<y+height+height*i)return i+1;
		return 0;
	}
	
	public:
	void set(GLint w,GLint h)
	{
		width=w;
		height=h;
		func=&empty;
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
	}
	void setcolors(GLfloat cr,GLfloat cg,GLfloat cb,GLfloat hcr,GLfloat hcg,GLfloat hcb,GLfloat ccr,GLfloat ccg,GLfloat ccb)
	{
		setarray3(color,cr,cg,cb);
		setarray3(hovercolor,hcr,hcg,hcb);
		setarray3(clickcolor,ccr,ccg,ccb);
	}
	void newoption(string str,GLint w,GLint h)
	{
		dropdowntype tempopt;
		tempopt.width=w;
		tempopt.height=h;
		tempopt.text=str;
		tempopt.enabled=-1;
		tempopt.func=&empty;
		option.push_back(tempopt);
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
	}
	void newoption(string str,GLint w,GLint h,void (*f)())
	{
		dropdowntype tempopt;
		tempopt.width=w;
		tempopt.height=h;
		tempopt.text=str;
		tempopt.enabled=-1;
		tempopt.func=f;
		option.push_back(tempopt);
	}
	void newmenu(GLint xloc,GLint yloc,GLint w,GLint h)
	{
		x=xloc;
		y=yloc;
		enabled=0;
	}	
	void disp()
	{
		int i,n;
		
		if(enabled>=0)
		{
			for(i=0;i<option.size();i++)
			{
				if(option[i].enabled)
				{
					depthbox(x,y+height*i,x+width,y+height*(i+1),clickcolor[0],clickcolor[1],clickcolor[2],.5,-1);
					option[i].disp();
				}				
				else
				{
					if(inrange(mouseloc[0],mouseloc[1]))
						depthbox(x,y+height*i,x+width,y+height*(i+1),hovercolor[0],hovercolor[1],hovercolor[2],.5,1);
					else depthbox(x,y+height*i,x+width,y+height*(i+1),color[0],color[1],color[2],.5,1);
				}
				glColor3f(0,0,0);
				dispstring(x+textoffset[0],y+height*(n+1)-textoffset[1],option[i].text);
			}
		}
	}
	bool click(GLint locx,GLint locy)
	{
		if(enabled)
		{
			if(option[enabled-1].inrange(locx,locy))
			{
				if(!option[enabled-1].click(locx,locy))
				{
					enabled=-1;
					return 0;
				}
				else return 1;
			}
			else if(inrange(locx,locy))
			{
				option[enabled-1].enabled=-1;
				enabled=inrange(locx,locy);
				return 1;
			}
		}
		else if(enabled=inrange(locx,locy))
		{
			if(option[enabled-1].option.size()==0)
			{
				func();
				enabled=-1;
				return 0;
			}
			else
			{
				option[enabled-1].newmenu(x+width,y+height*(enabled-1),width,height);
				return 1;
			}
		}
		enabled=-1;
		return 0;
	}
};
