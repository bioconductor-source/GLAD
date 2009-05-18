/*****************************************************************************/
/* Copyright (C) 2004 Institut Curie                                         */
/* Author(s): Philippe Hup� (Institut Curie) 2004                            */
/* Contact: glad@curie.fr                                                    */
/*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <map>

#include "glad-struct.h"
#include "glad-function.h"
#include "glad.h"


#ifndef MAXDOUBLE
#include <float.h>
#define MAXDOUBLE DBL_MAX
#endif

using namespace std;



extern "C"
{



  /*************************************/
  /* fonctions utilis�es dans filterBkp */
  /*************************************/

  void updateFilterBkp(const int *Chromosome,
		       int *Breakpoints,
		       int *Level,
		       const int *PosOrder,
		       double *NextLogRatio,
		       const double *LogRatio,
		       const int *maxLevel,
		       // ajout de variables pour updateOutliers
		       int *OutliersAws,
		       double *Smoothing,
		       // ajout de variables pour detectOutliers
		       int *OutliersMad,
		       int *OutliersTot,
		       const int *msize,
		       const double *alpha,
		       const int *l,
		       const double *NormalRef,
		       const double *deltaN,
		       int *NormalRange)
  {

    updateLevel(Chromosome,
    		Breakpoints,
    		Level,
    		PosOrder,
    		NextLogRatio,
    		LogRatio,
    		maxLevel,
    		l);


    updateOutliers (OutliersAws,
    		    Level,
    		    Breakpoints,
    		    Smoothing,
    		    l);


    detectOutliers(LogRatio,
    		   Level,
    		   OutliersAws,
    		   OutliersMad,
    		   OutliersTot,
    		   msize,
    		   alpha,
    		   l);

    //recalcul de la smoothing line
    compute_median_smoothing(LogRatio,
			     Level,
			     Smoothing,
			     l);

            
    // on prend comme r�f�rence ceux qui sont compris entre certaines valeurs            
    compute_NormalRange(Smoothing,
			NormalRef,
			Level,
			NormalRange,
			deltaN,
			l);



  }

  void updateLevel (const int *Chromosome,
		    const int *Breakpoints,
		    int *Level,
		    //		    int *Region,
		    const int *PosOrder,
		    double *NextLogRatio,
		    const double *LogRatio,
		    const int *maxLevel,
		    const int *l)
  {
    const int nb=*l;
    int pos_moins_un;
    int pos;
    int idLevel=*maxLevel;
 
    for (pos=1;pos<nb;pos++)
      {
	pos_moins_un=pos-1;
	if (Chromosome[pos]==Chromosome[pos_moins_un])
	  {
	    if (Breakpoints[pos_moins_un]!=1)
	      {
		Level[pos]=Level[pos_moins_un];
		//		Region[pos]=Region[pos_moins_un];
	      }
	    if (Breakpoints[pos_moins_un]==1)
	      {
		NextLogRatio[pos_moins_un]=LogRatio[pos];
		if (Level[pos_moins_un]==Level[pos])
		  {
		    idLevel++;
		    Level[pos]=idLevel;
		  }
	      }
	  }
      }
  }


  void updateOutliers (int *OutliersAws,
		       int *Level,
		       int *Breakpoints,
		       double *Smoothing,
		       const int *l)
  {
  
    int pos;
    int pos_moins_un;
    const int nb=*l-1;

    for (pos=1;pos<nb;pos++)
      {
	pos_moins_un=pos-1;
	if (Level[pos_moins_un]==Level[pos+1] && Level[pos_moins_un]!=Level[pos])
	  {
	    Level[pos]=Level[pos_moins_un];
	    Breakpoints[pos_moins_un]=0;
	    Breakpoints[pos]=0;
	    OutliersAws[pos]=1;
	    Smoothing[pos]=Smoothing[pos_moins_un];
	  }
      }

  }



  

  void updateBkpRL (int *Region,
		    int *OutliersAws,
		    int *Breakpoints,
		    const int *PosOrder,
		    double *NextLogRatio,
		    const double *LogRatio,
		    const int *l)
  {

    // le 03 01 09
    // la variables Chromosome a �t� supprim�e
    // car la fonction est utilis�e chromosome par chromosome
    int i;
    int i_moins_un;
    int i_plus_un;
    //    int i_moins_deux;
    const int nb=*l;
    const int nb_moins_un=*l-1;
    const int nb_moins_deux=*l-2;

    OutliersAws[0] = 0;
    Breakpoints[0] = 0;
    NextLogRatio[0] = 0;

    for (i=1;i<nb;i++)
      {

	OutliersAws[i] = 0;
	Breakpoints[i] = 0;
	NextLogRatio[i] = 0;

	i_moins_un=i-1;
	if (i==1 || i==nb_moins_un)
	  {
	    if (Region[i]!=Region[i_moins_un])
	      {
		if (i==1)
		  {
		    OutliersAws[0]=1;
		    Region[0]=Region[1];
		  }
		else
		  {
		    OutliersAws[nb_moins_un]=1;
		    Region[nb_moins_un]=Region[nb_moins_deux];
		  }
	      }
	  }
	else
	  {
	    i_plus_un=i+1;
	    // 	    if (Chromosome[i]!=Chromosome[i_moins_un])
	    // 	      {
	    // 		printf("Chromosomes diff�rents\n");
	    // 		i_moins_deux=i-2;
	    // 		if (Region[i_moins_un]!=Region[i_moins_deux])
	    // 		  {
	    // 		    Region[i_moins_un]=Region[i_moins_deux];
	    // 		    OutliersAws[i_moins_un]=1;
	    // 		  }

	    // 		if (Region[i_plus_un]!=Region[i])
	    // 		  {
	    // 		    Region[i_plus_un]=Region[i];
	    // 		    OutliersAws[i]=1;
	    // 		  }                
	    // 	      }
	    // 	    else
	    // 	      {
	    if (Region[i]!=Region[i_moins_un] && Region[i_plus_un]!=Region[i] && Region[i_plus_un]==Region[i_moins_un])
	      {
		if (OutliersAws[i_moins_un]==0)
		  {
		    OutliersAws[i]=1;
		    Region[i]=Region[i_moins_un];
		  }
	      }
	    else
	      {
		if (Region[i]!=Region[i_moins_un] && OutliersAws[i_moins_un]==0)
		  {
		    Breakpoints[i_moins_un]=1;
		    NextLogRatio[i_moins_un]=LogRatio[i];
		  }
	      }                                               
	    // 	      }
	  }
      }
  }





  void rangeGainLoss(const double *Smoothing,
		     const int *ZoneGNL,
		     const int *OutliersTot,
		     double *minG,
		     double *maxL,
		     double *minAmp,
		     double *maxDel,
		     const int *l)
  {
    int i;
    const int nb = *l;

    *minG = MAXDOUBLE;
    *minAmp = MAXDOUBLE;
    *maxL = -MAXDOUBLE;
    *maxDel = -MAXDOUBLE;

    for (i = 0; i < nb; i++)
      {
	if (OutliersTot[i] == 0)
	  {

	    switch(ZoneGNL[i])
	      {
	      case 0:
		break;

	      case 1:
		if(Smoothing[i] < *minG)
		  {
		    *minG = Smoothing[i];
		  }
		break;

	      case -1:
		if (Smoothing[i] > *maxL)
		  { 
		    *maxL = Smoothing[i]; 
		  }
		break;

	      case 2:
		if(Smoothing[i] < *minAmp)
		  {
		    *minAmp = Smoothing[i];
		  }
		break;

	      case -10:
		if(Smoothing[i] > *maxDel)
		  {
		    *maxDel = Smoothing[i];
		  }
		break;

	      }
	  }

      }
  }



  /*************************************/
  /* fonctions utilis�es dans daglad   */
  /*************************************/

  void delete_contiguous_bkp(int *BkpInfo_BkpToDel,
			     double *BkpInfo_Gap,
			     double *BkpInfo_LogRatio,
			     int *BkpInfo_NextPosOrder,
			     int *BkpInfo_PosOrder,
			     int *BkpInfo_Side,
			     double *BkpInfo_Sigma,
			     double *BkpInfo_Smoothing,
			     double *BkpInfo_SmoothingNext,
			     double *BkpInfo_Weight,
			     int *nb_Bkp,
			     int *RecomputeGNL,
			     const int *nbsigma)
  {

    // nb_Bkp=length(profileCGH$BkpInfo[,1])
    int i;
    int i_moins_un;
    const int l=*nb_Bkp;
    double UpLeft, LowLeft, UpRight, LowRight, DiffLeft, DiffRight;
    double LRV;
    double SigChr;


    //  for (i in 2:length(profileCGH$BkpInfo[,1]))
    for (i=1;i<l;i++)
      {
	i_moins_un=i-1;
	if (BkpInfo_PosOrder[i]==BkpInfo_NextPosOrder[i_moins_un] && BkpInfo_BkpToDel[i_moins_un]==0)
	  {
	    SigChr=BkpInfo_Sigma[i];
	    // on regarde d'abord � gauche
	    UpLeft=BkpInfo_Smoothing[i_moins_un] + 3*SigChr;
	    LowLeft=BkpInfo_Smoothing[i_moins_un] - 3*SigChr;

	    // On regarde ce qui se passe � droite
	    UpRight=BkpInfo_SmoothingNext[i] + 3*SigChr;
	    LowRight=BkpInfo_SmoothingNext[i] - 3*SigChr;

	    LRV=BkpInfo_LogRatio[i];
                    
	    if (((LRV > LowLeft) && (LRV < UpLeft)) || ((LRV > LowRight) && (LRV < UpRight)))
	      {
		*RecomputeGNL=1;
		if (((LRV > LowLeft) && (LRV < UpLeft)) && ((LRV > LowRight) && (LRV < UpRight)))
		  {
		    // attention, lors de la suppression d'un Bkp, le gap n'est plus bon
		    // d'o� recalcul du weight
		    // je ne vois pas o� il est recalcul�!!!!
		    DiffLeft=fabs(LRV - BkpInfo_Smoothing[i_moins_un]);
		    DiffRight=fabs(LRV - BkpInfo_SmoothingNext[i]);
		    if (DiffRight<DiffLeft)
		      {
			// On fusionne le Bkp avec la r�gion � droite
			BkpInfo_BkpToDel[i]=1;
			BkpInfo_Side[i]=1;
			BkpInfo_Gap[i_moins_un]=fabs(BkpInfo_Smoothing[i_moins_un]-BkpInfo_SmoothingNext[i]);
			BkpInfo_Weight[i_moins_un]=1 - kernelpen(BkpInfo_Gap[i_moins_un], *nbsigma*BkpInfo_Sigma[i_moins_un]);
                                
		      }
		    else
		      {
			// On fusionne le Bkp avec la r�gion � gauche
			BkpInfo_BkpToDel[i_moins_un]=1;
			BkpInfo_Side[i_moins_un]=0;
			BkpInfo_Gap[i]=fabs(BkpInfo_Smoothing[i_moins_un]-BkpInfo_SmoothingNext[i]);
			BkpInfo_Weight[i]=1 - kernelpen(BkpInfo_Gap[i_moins_un], *nbsigma*BkpInfo_Sigma[i_moins_un]);
                                

		      }
		  }
		else
		  {
		    if (((LRV > LowLeft) && (LRV < UpLeft)))
		      {
			// On fusionne le Bkp avec la r�gion � gauche
			BkpInfo_BkpToDel[i_moins_un]=1;
			BkpInfo_Side[i_moins_un]=0;
			BkpInfo_Gap[i]=fabs(BkpInfo_Smoothing[i_moins_un]-BkpInfo_SmoothingNext[i]);
			BkpInfo_Weight[i]=1 - kernelpen(BkpInfo_Gap[i_moins_un], *nbsigma*BkpInfo_Sigma[i_moins_un]);
                                
		      }
		    else
		      {
			// On fusionne le Bkp avec la r�gion � droite
			BkpInfo_BkpToDel[i]=1;
			BkpInfo_Side[i]=1;
			BkpInfo_Gap[i_moins_un]=fabs(BkpInfo_Smoothing[i_moins_un]-BkpInfo_SmoothingNext[i]);
			BkpInfo_Weight[i_moins_un]=1 - kernelpen(BkpInfo_Gap[i_moins_un], *nbsigma*BkpInfo_Sigma[i_moins_un]);                            
		      }
		  }                                        
	      }
	  }
      }
  }




  /*************************************/
  /* fonctions utilis�es dans BkpInfo.R  */
  /*************************************/

  void make_BkpInfo(const double *BkpInfo_Gap,
		    int *BkpInfo_GNLchange,
		    const double *BkpInfo_Value,
		    double *BkpInfo_Weight,
		    int *BkpInfo_ZoneGNL,
		    const int *BkpInfo_ZoneGNLnext,
		    const int *nb_Bkp,
		    const double *nbsigma)
  {

    const int l=*nb_Bkp;
    int i;

    for (i=0;i<l;i++)
      {
	BkpInfo_Weight[i]=1 - kernelpen(BkpInfo_Gap[i], *nbsigma*BkpInfo_Value[i]);
	if (BkpInfo_ZoneGNL[i]==BkpInfo_ZoneGNLnext[i])
	  {
	    BkpInfo_GNLchange[i]=0;
	  }
	else
	  {
	    BkpInfo_GNLchange[i]=1;
	  }
      }
  }


  /*******************************************/
  /* fonctions utilis�es dans testMoveBkp.R  */
  /*******************************************/

  void loopTestBkpToMove(const double *LogRatio,
			 const double *NextLogRatio,
			 const double *Smoothing,
			 const double *SmoothingNext,
			 const int *PosOrder,
			 const int *MaxPosOrder,
			 const int *MinPosOrder,
			 int *MoveBkp,
			 const int *NbBkp)
  {
    int i;
    int l=*NbBkp;

    for (i=0;i<l;i++)
      {
	MoveBkp[i]=testSingle(LogRatio[i],NextLogRatio[i],Smoothing[i],SmoothingNext[i]);

	if(MoveBkp[i]==1)
	  {
	    if((PosOrder[i]+1)==MaxPosOrder[i])
	      {
		MoveBkp[i]=0;
	      }
	  }
	else
	  {
	    if(MoveBkp[i]==-1)
	      {
		if((PosOrder[i]-1)==MinPosOrder[i])
		  {
		    MoveBkp[i]=0;
		  }
	      }
	  }
      }
  }



  int  testSingle(const double LogRatio,
		  const double NextLogRatio,
		  const double Smoothing,
		  const double SmoothingNext)
  {
    int moveBkp=0;
    // le cr�neau est plus bas � droite qu'� gauche
    if (Smoothing > SmoothingNext)
      {
	if ((SmoothingNext <= LogRatio) && (LogRatio <= Smoothing))
	  {
	    if((LogRatio-SmoothingNext) < (Smoothing-LogRatio))
	      {
		// il faut d�placer le Bkp vers la gauche
		moveBkp=-1;
	      }                        
	  }
            
	if ((SmoothingNext <= NextLogRatio) && (NextLogRatio <= Smoothing))
	  {
	    if ( (NextLogRatio-SmoothingNext)>(Smoothing-NextLogRatio))
	      {
		// il faut d�placer le Bkp vers la droite
		moveBkp=1;
	      }
                
	  }

	if (LogRatio <= SmoothingNext)
	  {
	    moveBkp=-1;
	  }

	if (NextLogRatio>=Smoothing)
	  {
	    moveBkp=1;
	  }
      }
    // le cr�neau est plus bas � gauche qu'� droite
    else
      {
	if ((SmoothingNext >= LogRatio) && (LogRatio >= Smoothing))
	  {
	    if ((SmoothingNext-LogRatio) < (LogRatio - Smoothing))
	      {
		// il faut d�placer le Bkp vers la gauche
		moveBkp=-1;
	      }
	  }

	if ((SmoothingNext >= NextLogRatio) & (NextLogRatio >= Smoothing))
	  {
	    if ((SmoothingNext-NextLogRatio) > (NextLogRatio-Smoothing))
	      {
		// il faut d�placer le Bkp vers la droite
		moveBkp=1;
	      }
	  }

	if (LogRatio>=SmoothingNext)
	  {
	    moveBkp=-1;
	  }

	if (NextLogRatio<=Smoothing)
	  {
	    moveBkp=1;
	  }
      }
    return(moveBkp);

  }


  /*******************************************/
  /* fonctions utilis�es dans OutliersGNL.R  */
  /*******************************************/

  void OutliersGNL(int * OutliersTot,
		   int *ZoneGNL,
		   const double *LogRatio,
		   const double * Smoothing,
		   const double *seuilsupValue,
		   const double *seuilinfValue,
		   const double *ampliconValue,
		   const double *deletionValue,
		   const double *NormalRefValue,
		   const int *l)
  {
    int i;
    const int nb=*l;

    const double seuilsup=*seuilsupValue;
    const double seuilinf=*seuilinfValue;
    const double amplicon=*ampliconValue;
    const double deletion=*deletionValue;
    const double NormalRef=*NormalRefValue;

    int checkGain=0;
    int checkLost=0;
    int checkNormal=0;
    int checkAlert=0;

    double LogRatio_moins_NormalRef;
    double minNormal=MAXDOUBLE;
    double maxNormal=-MAXDOUBLE;
    double minGain=MAXDOUBLE;
    double maxLost=-MAXDOUBLE;

    /*   printf("seuilsup=%f\n",seuilsup); */
    /*   printf("seuilinf=%f\n",seuilinf); */
    /*   printf("amplicon=%f\n",amplicon); */
    /*   printf("deletion=%f\n",deletion); */

    for (i=0;i<nb;i++)
      {
	//////////////////////////
	// On regarde les Outliers
	//////////////////////////
	if(OutliersTot[i]!=0)
	  {
	    // On met le GNL de tous les Outliers � 0
	    ZoneGNL[i]=0;

	    // Calcul de la diff�rence entre le LogRatio et NormalRef
	    if(NormalRef!=0)
	      {
		LogRatio_moins_NormalRef=LogRatio[i]-NormalRef;
	      }
	    else
	      {
		LogRatio_moins_NormalRef=LogRatio[i];
	      }

	    // Gain et Amplicon
	    if(LogRatio_moins_NormalRef>seuilsup)
	      {
		// On a un Amplicon
		if(LogRatio_moins_NormalRef >= amplicon)
		  {
		    ZoneGNL[i]=2;
		  }
		// On a un Gain
		else
		  {
		    ZoneGNL[i]=1;
		  }
	      }
	    // Perte et Deletion
	    else
	      {
		if(LogRatio_moins_NormalRef<seuilinf)
		  {
		    // On a une deletion
		    if(LogRatio_moins_NormalRef<deletion)
		      {
			ZoneGNL[i]=-10;
		      }
		    // On a une Perte
		    else
		      {
			ZoneGNL[i]=-1;
		      }
		  }
	      }
	  }
	/////////////////////////////////////////////////////
	// R�cup�ration des min/max pour le Normal/Gain/Perte
	/////////////////////////////////////////////////////
	else
	  {
	    switch(ZoneGNL[i])
	      {
	      case 0:
		if(Smoothing[i]<minNormal)
		  {
		    minNormal=Smoothing[i];
		  }
		if(Smoothing[i]>maxNormal)
		  {
		    maxNormal=Smoothing[i];
		  }
		checkNormal=1;
		break;

	      case 1:
		if(Smoothing[i]<minGain)
		  {
		    minGain=Smoothing[i];
		  }
		checkGain=1;
		break;

	      case -1:
		if(Smoothing[i]>maxLost)
		  {
		    maxLost=Smoothing[i];
		  }
		checkLost=1;
		break;
	      }
	  }
      }


    ////////////////////////////////////////////////////////////////////
    // On fait une seconde boucle pour mettre � jour le GNL des Outliers 
    // Et v�rifier la coh�rence des valeurs
    ////////////////////////////////////////////////////////////////////

    for(i=0;i<nb;i++)
      {
	//////////////////////////
	// On regarde les Outliers
	//////////////////////////
	if(OutliersTot[i]!=0)
	  {
	    if(ZoneGNL[i]==0)
	      {
		// comparaison avec minGain et maxLost
		if(LogRatio[i]>minGain)
		  {
		    ZoneGNL[i]=1;
		  }
		else
		  {
		    if(LogRatio[i]<maxLost)
		      {
			ZoneGNL[i]=-1;
		      }
		  }
	      }
	  }
	else
	  {
	    if(checkLost && checkGain)
	      {
		if(checkNormal)
		  {
		    if (maxLost>minNormal)
		      {
			if(Smoothing[i]<=maxLost && ZoneGNL[i]==0)
			  {
			    ZoneGNL[i]=-1;
			    checkAlert=1;
			  }
		      }
		    if(minGain<minNormal)
		      {
			if(Smoothing[i]>=minGain && ZoneGNL[i]==0)
			  {
			    ZoneGNL[i]=1;
			    checkAlert=1;
			  }
		      }
		  }
	      }
	  }
      }

    if (maxLost>minGain)
      {
	checkAlert=1;
      }

    if (checkAlert)
      {
	printf("In function OutliersGNL: Inconsistency for smoothing values vs. GNL status has been corrected)\n");
      }
  }




  /********************************************************************/
  /* fonction pour faire une aggregation avec le calcul de la m�dian  */
  /********************************************************************/


  void compute_median_smoothing(const double *LogRatio,
				const int *ByValue,
				double *Smoothing,
				const int *l)
  {
    int i,j;
    const int nb=*l;
    double *median_ByValue;
    int *unique_ByValue;
    int nb_unique_ByValue;

    map<int, vector<double> > agg_LogRatio;
    map<int, vector<double> >::iterator it_agg_LogRatio;

    for(i = 0; i < nb; i++)
      {
	agg_LogRatio[ByValue[i]].push_back(LogRatio[i]);
      }


    median_ByValue = (double *)malloc(agg_LogRatio.size() * sizeof(double));
    unique_ByValue = (int *)malloc(agg_LogRatio.size() * sizeof(int));
    it_agg_LogRatio=agg_LogRatio.begin();

    for (j = 0; j < (int)agg_LogRatio.size(); j++)
      {
	median_ByValue[j] = quantile_vector_double(it_agg_LogRatio->second, 0.5);
	unique_ByValue[j] = it_agg_LogRatio->first;

	it_agg_LogRatio++;

      }

    nb_unique_ByValue = (int)agg_LogRatio.size();
    my_merge(ByValue,
	     Smoothing,
	     unique_ByValue,
	     median_ByValue,
	     l,
	     &nb_unique_ByValue);




    free(median_ByValue);
    free(unique_ByValue);
  }



  /**********************************************************/
  /* fonction pour le calcul des clusters Loss/Normal/Gain  */
  /**********************************************************/


  void compute_cluster_LossNormalGain(// variables pour faire la jointure
				      const int *ZoneGen,
				      int *value_dest,
				      const int *length_dest,
				      const double *Smoothing,
				      const double *forceGL1Value,
				      const double *forceGL2Value,
				      const double *NormalRefValue,
				      const double *ampliconValue,
				      const double *deletionValue,
				      //variables pour calcul la m�diane par cluster
				      const double *LogRatio,
				      const int *NormalRange)
  {

    int i,j;
    int nb = *length_dest;
    int NormalCluster = 0;
    int NormalCluster_not_detected = 1;

    int *MedianCluster_ZoneGen;
    int *MedianCluster_ZoneGNL;
    double *MedianCluster_Median;
    int nb_unique_ZoneGen;

    double RefNorm = 0;
    vector<int>::iterator it_new_end_NormalCluster;


    map<int, vector<double> > agg_LogRatio;
    map<int, vector<double> >::iterator it_agg_LogRatio;


    // On r�cup�re les valeurs de LogRatio pour chaque ZoneGen
    for (i = 0; i < nb; i++)
      {
	agg_LogRatio[ZoneGen[i]].push_back(LogRatio[i]);

	// le cluster correspondant au normal est celui qui comprend
	// le NormalRange 0
	if(NormalRange[i] == 0 && NormalCluster_not_detected)
	  {
	    NormalCluster = ZoneGen[i];
	    NormalCluster_not_detected = 0;
	  }
      }

    // On calcule la m�diane par ZoneGen
    MedianCluster_Median = (double *)malloc(agg_LogRatio.size() * sizeof(double));
    MedianCluster_ZoneGen = (int *)malloc(agg_LogRatio.size() * sizeof(int));
    MedianCluster_ZoneGNL = (int *)malloc(agg_LogRatio.size() * sizeof(int));
    it_agg_LogRatio=agg_LogRatio.begin();

    for (j = 0; j < (int)agg_LogRatio.size(); j++)
      {
	MedianCluster_Median[j] = quantile_vector_double(it_agg_LogRatio->second, 0.5);
	MedianCluster_ZoneGen[j] = it_agg_LogRatio->first;

	if(NormalCluster == it_agg_LogRatio->first)
	  {
	    RefNorm = MedianCluster_Median[j];
	  }

	it_agg_LogRatio++;

      }

    for (j = 0; j < (int)agg_LogRatio.size(); j++)
      {
	MedianCluster_ZoneGNL[j] = 0;

	if(MedianCluster_Median[j] > RefNorm)
	  {
	    MedianCluster_ZoneGNL[j] = 1;
	  }
	else
	  {
	    if(MedianCluster_Median[j] < RefNorm)
	      {
		MedianCluster_ZoneGNL[j] = -1;
	      }
	  }
      }

    nb_unique_ZoneGen = (int)agg_LogRatio.size();
    my_merge_int_forceGL(ZoneGen,
			 value_dest,
			 MedianCluster_ZoneGen,
			 MedianCluster_ZoneGNL,
			 length_dest,
			 &nb_unique_ZoneGen,
			 Smoothing,
			 forceGL1Value,
			 forceGL2Value,
			 NormalRefValue,
			 ampliconValue,
			 deletionValue);


    free(MedianCluster_ZoneGen);
    free(MedianCluster_Median);
    free(MedianCluster_ZoneGNL);

  }

  void compute_NormalRange(const double *Smoothing,
			   const double *NormalRef,
			   const int *Level,
			   int *NormalRange,
			   const double *deltaN,
			   const int *l)
  {

    int i;
    const int nb = *l;

    for (i = 0; i < nb; i++)
      {
	if(fabs(Smoothing[i] - *NormalRef) <= *deltaN)
	  {
	    NormalRange[i] = 0;
	  }
	else
	  {
	    NormalRange[i] = Level[i];
	  }
      }

  }

}





