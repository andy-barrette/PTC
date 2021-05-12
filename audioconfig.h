// Compile as:
// g++ -o name filename.cpp -lasound

#include<iostream>
#include<stdio.h>
#include<string.h>
#include<cstring>
#include<alsa/asoundlib.h>
#include<math.h>
#include<fstream>
using namespace std;

class audiohwtype
{
	public:
	int card,dev,subdev,cardcount,devcount,subdevcount;
	snd_ctl_t *cardhandle;
	snd_pcm_t *pcmhandle;
	snd_pcm_info_t *pcminfo;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_uframes_t buffersize;
	int rate,channels;
	char hwname[64];
	
	void setname(int card,int device,int subdevice)
	{
		int i;
		string str="hw:";
		str+=(char)(card+48);
		str+=',';
		str+=(char)(device+48);
		str+=',';
		str+=(char)(subdevice+48);
		for(i=0;i<64;i++)
		{
			if(i<str.size())hwname[i]=str[i];
			else hwname[i]='\0';
		}
	}			
};

bool dlhwinfo(char *filename,audiohwtype *aset)
{
	char temp[64];
	char a;
	int i,err;
	FILE *hwinfofile;
	
	if((hwinfofile=fopen(filename,"r"))==NULL)return false;
	
	a=getc(hwinfofile);
	for(i=0,a=getc(hwinfofile);a!=' ';i++,a=getc(hwinfofile))
		aset->hwname[i]=a;
	aset->hwname[i]='\0';
	
	for(i=0,a=getc(hwinfofile);a!=' ';i++,a=getc(hwinfofile))
		temp[i]=a;
	temp[i]='\0';
	aset->card=atoi(temp);
	
	for(i=0,a=getc(hwinfofile);a!=' ';i++,a=getc(hwinfofile))
		temp[i]=a;
	temp[i]='\0';
	aset->dev=atoi(temp);
	
	for(i=0,a=getc(hwinfofile);a!=' ';i++,a=getc(hwinfofile))
		temp[i]=a;
	temp[i]='\0';
	aset->subdev=atoi(temp);
	
	for(i=0,a=getc(hwinfofile);a!=' ';i++,a=getc(hwinfofile))
		temp[i]=a;
	temp[i]='\0';
	aset->cardcount=atoi(temp);
	
	for(i=0,a=getc(hwinfofile);a!=' ';i++,a=getc(hwinfofile))
		temp[i]=a;
	temp[i]='\0';
	aset->devcount=atoi(temp);
	
	for(i=0,a=getc(hwinfofile);a!=' ';i++,a=getc(hwinfofile))
		temp[i]=a;
	temp[i]='\0';
	aset->subdevcount=atoi(temp);
	
	fclose(hwinfofile);
	if((err=snd_ctl_open(&aset->cardhandle,aset->hwname,0))<0)
	{
		printf("Can't open card %i: %s\n", aset->card, snd_strerror(err));
		return false;
	}
	snd_pcm_info_alloca(&aset->pcminfo);
	memset(aset->pcminfo,0,snd_pcm_info_sizeof());

	snd_pcm_info_set_device(aset->pcminfo,aset->dev);
	snd_pcm_info_set_stream(aset->pcminfo,SND_PCM_STREAM_PLAYBACK);
	snd_pcm_info_set_subdevice(aset->pcminfo, aset->subdev);
	return true;
}

void ulhwinfo(char *filename,audiohwtype *aset)
{
	char temp[64];
	FILE *hwinfofile;
	fopen(filename,"w+");
	
	fputs(aset->hwname,hwinfofile);
	fprintf(hwinfofile," %i",aset->card);
	fprintf(hwinfofile," %i",aset->dev);
	fprintf(hwinfofile," %i",aset->subdev);
	fprintf(hwinfofile," %i",aset->cardcount);
	fprintf(hwinfofile," %i",aset->devcount);
	fprintf(hwinfofile," %i",aset->subdevcount);
}

int set_audio_hardware(audiohwtype *aset,unsigned int rate,int channels)
{
	int err;
	aset->rate=rate;
	aset->channels=channels;
	sprintf(aset->hwname,"hw:%i",aset->card);
	if(aset->subdevcount>1)
		sprintf(aset->hwname,"hw:%i,%i,%i",aset->card,aset->dev,aset->subdev);
	else
		sprintf(aset->hwname,"hw:%i,%i",aset->card,aset->dev);
	if((err=snd_pcm_open(&aset->pcmhandle,aset->hwname,SND_PCM_STREAM_PLAYBACK,0))<0)
	{
		printf("\nCan't open wave output: %s\n",snd_strerror(err));
		return 0;
	}
	
	if((err=snd_pcm_hw_params_malloc(&aset->hwparams))<0)
	{
     		printf("Can't get sound hardware struct %s\n",snd_strerror(err));
		bad1: return(err);
  	}
	if((err=snd_pcm_hw_params_any(aset->pcmhandle,aset->hwparams))<0)
	{
		printf("Can't init sound hardware struct: %s\n",snd_strerror(err));
		bad2: snd_pcm_hw_params_free(aset->hwparams);
		goto bad1;
   	}
	if((err=snd_pcm_hw_params_set_format(aset->pcmhandle,aset->hwparams,SND_PCM_FORMAT_S16_LE))<0)
	{
		printf("Can't set 16-bit: %s\n",snd_strerror(err));
		goto bad2;
	}
	if((err=snd_pcm_hw_params_set_rate(aset->pcmhandle,aset->hwparams,rate,0))<0)
	{
		printf("Can't set sample rate: %s\n",snd_strerror(err));
		goto bad2;
	}
	if((err=snd_pcm_hw_params_set_channels(aset->pcmhandle,aset->hwparams,channels))<0)
	{
		printf("Can't set number of channels: %s\n",snd_strerror(err));
		goto bad2;
	}
	if((err=snd_pcm_hw_params_set_access(aset->pcmhandle,aset->hwparams,SND_PCM_ACCESS_RW_INTERLEAVED))<0)
	{
		printf("Can't set number access mode: %s\n",snd_strerror(err));
		goto bad2;
	}
	if ((err=snd_pcm_hw_params(aset->pcmhandle,aset->hwparams))<0)
	{
		printf("Can't set hardware params: %s\n",snd_strerror(err));
		goto bad2;
	}
	
	snd_pcm_hw_params_free(aset->hwparams);
	snd_pcm_close(aset->pcmhandle);
	return(0);
}


int testdevice(audiohwtype *aset)
{
	cout<<"\nTesting "<<aset->hwname;
	int err,result;
	float a=30000,f=1000,n=10000,c;//amplitude, frequency, total number of samples,current sample
	
	if((err=snd_pcm_open(&aset->pcmhandle,aset->hwname,SND_PCM_STREAM_PLAYBACK,0))<0)
	{
		printf("\nCan't open wave output: %s\n",snd_strerror(err));
		return 0;
	}
	else
	{
		//set_audio_hardware(aset,44100,2);
		snd_pcm_set_params(aset->pcmhandle,SND_PCM_FORMAT_S16_LE,SND_PCM_ACCESS_RW_INTERLEAVED,2,44100,1,1000);
		short *buffer;
		buffer=(short*)malloc(2*n);
		for(c=0;c<n*.6;c++)
		{
			buffer[2*(int)c]=(short)(a*sin(f*c/44100.0));
			buffer[2*(int)c+1]=(short)(a*sin(f*c/44100.0));
		}
		if(snd_pcm_writei(aset->pcmhandle,buffer,100)<0)snd_pcm_prepare(aset->pcmhandle);
		snd_pcm_close(aset->pcmhandle);
		free(buffer);
		cout<<"\nDid you hear a sound? (\"1\"=yes, \"0\"=no)\t";
		cin>>result;
		return result;
	}
}

int scanalldev(audiohwtype *aset,bool test,bool fileio)
{
	int  err,found=0,i;
	char fname[64];
	cout<<"\nStarting audio test...";
	aset->card=-1;
	while(found==0)
	{
		if((err=snd_card_next(&aset->card))<0)
		{
			printf("Can't get the next card number: %s\n",snd_strerror(err));
			break;
		}

		if(aset->card<0)
		{
			printf("No sound card found\n");
			return 0;
		}

		sprintf(aset->hwname,"hw:%i",aset->card);
		if((err=snd_ctl_open(&aset->cardhandle,aset->hwname,0))<0)
		{
			printf("Can't open card %i: %s\n",aset->card,snd_strerror(err));
			continue;
		}

		aset->dev=-1;	
		while(found==0)
		{
			if((err=snd_ctl_pcm_next_device(aset->cardhandle,&aset->dev))<0)
			{
				printf("Can't get next wave device number: %s\n", snd_strerror(err));
				break;
			}
			if(aset->dev<0)break;

			snd_pcm_info_alloca(&aset->pcminfo);
			memset(aset->pcminfo,0,snd_pcm_info_sizeof());

			snd_pcm_info_set_device(aset->pcminfo,aset->dev);
			snd_pcm_info_set_stream(aset->pcminfo,SND_PCM_STREAM_PLAYBACK);

			i=-1;
			aset->subdevcount=1;

			while (++i<aset->subdevcount)
			{
				snd_pcm_info_set_subdevice(aset->pcminfo, i);
				if ((err=snd_ctl_pcm_info(aset->cardhandle,aset->pcminfo))<0)
				{
					printf("Can't get info for wave output subdevice hw:%i,%i,%i: %s\n", aset->card, aset->dev, i, snd_strerror(err));
					continue;
				}
				if(!i)
				{
					aset->subdevcount=snd_pcm_info_get_subdevices_count(aset->pcminfo);
					printf("\nFound %i wave output subdevices on card %i\n",aset->subdevcount,aset->card);
				}
				if(aset->subdevcount>1)
					sprintf(aset->hwname,"hw:%i,%i,%i",aset->card,aset->dev,i);
				else
					sprintf(aset->hwname,"hw:%i,%i",aset->card,aset->dev);
				if(test)
				{
					if(testdevice(aset)==1)
					{
						found++;
						if(fileio)
						{
							strcpy(fname,"hwconfig.txt");
							ulhwinfo(fname,aset);
						}
					}
				}
				else
				{
					found++;
					if(fileio)
					{
						strcpy(fname,"hwconfig.txt");
						ulhwinfo(fname,aset);
					}
				}
			}
		}
	}
	snd_ctl_close(aset->cardhandle);
	snd_config_update_free_global();
	cout<<"\n"<<found<<" output-ready devices were found.\n";
	return found;
}
