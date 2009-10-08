// $Date$ 
// $Revision$
// $Author$

#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svdiff.h"
#include "MALLOC.h"

// svn revision character string
char IOdplot_revision[]="$Revision$";

/* ------------------ setup_plot3d ------------------------ */

void setup_plot3d(FILE *stream_out){
  casedata *case1, *case2;
  int i;

  case1 = caseinfo;
  case2 = caseinfo + 1;

  for(i=0;i<case1->nplot3d_files;i++){
    plot3d *plot3di;

    plot3di = case1->plot3dinfo + i;
    plot3di->plot3d2 = getplot3d(plot3di,case2);
    if(plot3di->plot3d2!=NULL&&stream_out!=NULL){
      char outfile[1024];
      int j;

      fprintf(stream_out,"%s\n",plot3di->keyword);
      make_outfile(outfile,NULL,plot3di->file,".q");
      fprintf(stream_out,"%s\n",outfile);
      for(j=0;j<5;j++){
        flowlabels *label;

        label = plot3di->labels + j;
        fprintf(stream_out,"%s\n",label->longlabel);
        fprintf(stream_out,"%s\n",label->shortlabel);
        fprintf(stream_out,"%s\n",label->unit);
      }
    }
  }
}

/* ------------------ getplot3d ------------------------ */

plot3d *getplot3d(plot3d *plot3din, casedata *case2){
  int i;
  float dx, dy, dz;
  mesh *meshin;

  meshin = plot3din->plot3dmesh;
  dx = meshin->dx/2.0;
  dy = meshin->dy/2.0;
  dz = meshin->dz/2.0;

  for(i=0;i<case2->nplot3d_files;i++){
    plot3d *plot3dout;
    mesh *meshout;

    plot3dout = case2->plot3dinfo + i;
    meshout = plot3dout->plot3dmesh;
    if(fabs(plot3din->time-plot3dout->time)>0.01)continue;
    if(strcmp(plot3din->labels[0].longlabel,plot3dout->labels[0].longlabel)!=0)continue;
    if(strcmp(plot3din->labels[1].longlabel,plot3dout->labels[1].longlabel)!=0)continue;
    if(strcmp(plot3din->labels[2].longlabel,plot3dout->labels[2].longlabel)!=0)continue;
    if(strcmp(plot3din->labels[3].longlabel,plot3dout->labels[3].longlabel)!=0)continue;
    if(strcmp(plot3din->labels[4].longlabel,plot3dout->labels[4].longlabel)!=0)continue;
    if(fabs(meshin->xbar0-meshout->xbar0)>dx)continue;
    if(fabs(meshin->xbar-meshout->xbar)>dx)continue;
    if(fabs(meshin->ybar0-meshout->ybar0)>dy)continue;
    if(fabs(meshin->ybar-meshout->ybar)>dy)continue;
    if(fabs(meshin->zbar0-meshout->zbar0)>dz)continue;
    if(fabs(meshin->zbar-meshout->zbar)>dz)continue;
    return plot3dout;
  }
  return NULL;
}

/* ------------------ diff_plot3ds ------------------------ */

void diff_plot3ds(void){
  int j;
  char *file1, *file2;
  char fullfile1[1024], fullfile2[1024], outfile[1024];

  for(j=0;j<caseinfo->nplot3d_files;j++){
    plot3d *plot3di, *plot3d1, *plot3d2;
    float *qframe1, *qframe2, *qout;
    mesh *plot3dmesh;
    int nx, ny, nz, nq;
    int len1, len2, lenout;
    int isotest=0;
    FILE *stream;
    int error1, error2, error3;
    int endian;
    int i;

    endian = getendian();
    plot3di = caseinfo->plot3dinfo+j;
    plot3d1 = plot3di;
    if(plot3di->plot3d2==NULL)continue;
    plot3d2 = plot3di->plot3d2;
    file1 = plot3d1->file;
    file2 = plot3d2->file;
    fullfile(fullfile1,sourcedir1,file1);
    fullfile(fullfile2,sourcedir2,file2);

    stream=fopen(fullfile1,"r");
    if(stream==NULL)continue;
    fclose(stream);

    stream=fopen(fullfile2,"r");
    if(stream==NULL)continue;
    fclose(stream);

    make_outfile(outfile,destdir,file1,".q");
    if(strlen(outfile)==0)continue;
    stream=fopen(outfile,"w");
    if(stream==NULL)continue;
    fclose(stream);


    plot3dmesh = plot3d1->plot3dmesh;
    nx = plot3dmesh->ibar+1;
    ny = plot3dmesh->jbar+1;
    nz = plot3dmesh->kbar+1;

    nq = 5*nx*ny*nz;
    NewMemory((void **)&qframe1,nq*sizeof(float));
    NewMemory((void **)&qframe2,nq*sizeof(float));
    NewMemory((void **)&qout,nq*sizeof(float));

    len1=strlen(fullfile1);
    len2=strlen(fullfile2);
    lenout=strlen(outfile);
    isotest=0;
    printf("subtracting %s from %s\n",fullfile1,fullfile2);
    if(test_mode==1)isotest=1;
    FORTgetplot3dq(fullfile1,&nx,&ny,&nz,qframe1,&error1,&endian,&isotest,len1);
    if(test_mode==1)isotest=2;
    FORTgetplot3dq(fullfile2,&nx,&ny,&nz,qframe2,&error2,&endian,&isotest,len2);
    for(i=0;i<nq;i++){
      qout[i]=qframe2[i]-qframe1[i];
    }
    FORTplot3dout(outfile,&nx,&ny,&nz,qout,&error3,lenout);
    FREEMEMORY(qframe1);
    FREEMEMORY(qframe2);
    FREEMEMORY(qout);
  }
}
