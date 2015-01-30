

#include <iostream>
#include <vector>
#include "oaDesignDB.h"

#include "/w/class/ee201a/ee201ata/oa/examples/oa/common/commonTechObserver.h"
#include "/w/class/ee201a/ee201ata/oa/examples/oa/common/commonLibDefListObserver.h"
#include "/w/class/ee201a/ee201ata/oa/examples/oa/common/commonFunctions.h"

#include <stdio.h>
#include <stdlib.h>
#include "./MisalignInst.h"
#include "./MisalignRow.h"

#include <cmath>


using namespace oa;
using namespace std;

static oaNativeNS ns;

//-------helper function for qsort-----------------------
int compareRow(const void * r1, const void * r2){
	return *(int*) r1-*(int*) r2;
}

int compareRow_surplus(const void * rr1, const void * rr2){
	MisalignRow r1 = *(MisalignRow*) rr1;
	MisalignRow r2 = *(MisalignRow*) rr2;
	if(r2.surplusonRow==r1.surplusonRow){return r1.y-r2.y;}
	else{return r2.surplusonRow-r1.surplusonRow;}
}

int compareRow_y(const void * rr1, const void * rr2){
	MisalignRow r1 = *(MisalignRow*) rr1;
	MisalignRow r2 = *(MisalignRow*) rr2;
	return r1.y - r2.y;
}

int compareInst_x(const void * ir1, const void * ir2){
	MisalignInst i1 = *(MisalignInst*) ir1;
	MisalignInst i2 = *(MisalignInst*) ir2;
	return i1.x - i2.x;
}

int compareInst_y(const void * ir1, const void * ir2){
	MisalignInst i1 = *(MisalignInst*) ir1;
	MisalignInst i2 = *(MisalignInst*) ir2;
	if( i1.row == i2.row){
		return i1.x - i2.x;
	}
	else{
		return  i1.row -  i2.row;
	}
}

int compareInst_surplus(const void * ir1, const void * ir2){
	MisalignInst i1 = *(MisalignInst*) ir1;
	MisalignInst i2 = *(MisalignInst*) ir2;
	if( i1.surplus == i2.surplus){
		if(i1.y==i2.y){return i1.x - i2.x;}
		else{return i1.y-i2.y;}
	}
	else{
		return  i2.surplus - i1.surplus;
	}
}


void misalignment_removal(oaBlock* block, MisalignRow* Rows, MisalignInst* Insts, int numRows, int numInsts, int &leftbound, int &rightbound){
	//---------------misalignment removal------------------------------------------------
	//    	int* RowsY = (int*) malloc(sizeof(int)*numRows);
	int i=-1;//Counter
	oaIter<oaRow> rowIter(block->getRows());
	cout<<"num Rows: "<<numRows<<endl;
	while( oaRow* RowP = rowIter.getNext()){
		i++;
		Rows[i].saveRow(RowP);
	}
	leftbound = Rows[0].x1;
	rightbound = Rows[0].x2;

	oaIter<oaInst> instIter(block->getInsts());
	i=0;//Counter

	while(  oaInst* InstP = instIter.getNext()){

		Insts[i].saveInst(InstP);
		//Locate the row number using binary search
		int low=0, current=numRows/2, high=numRows-1;
		while(low<=high){
			if(Insts[i].y == Rows[current].y){
				break;
			}
			else{
				if(Insts[i].y < Rows[current].y){
					high = current-1;
					current=(low + high)/2;
				}
				else{
					low = current+1;
					current = (low + high) /2;
				}
			}
		}
		if(Insts[i].x1<leftbound){
			Insts[i].MoveRight(leftbound-Insts[i].x1);
		}
		if(Insts[i].x2>rightbound){
			Insts[i].MoveRight(rightbound-Insts[i].x2);
		}
		if(low>high){
			int neighborRowY_a = Rows[low].y;
			int neighborRowY_b = Rows[high].y;
			if(abs(Insts[i].y-neighborRowY_a)<abs(Insts[i].y-neighborRowY_b)){
				Insts[i].SetCenterCoord(Insts[i].x,neighborRowY_a);
			}
			else{
				Insts[i].SetCenterCoord(Insts[i].x,neighborRowY_b);
			}
		}
		i++;
	}
}


typedef std::vector<MisalignInst*> InstVec;
int meet_row_capacity_constraints(oaBlock* block, MisalignRow* Rows, MisalignInst* Insts, int numRows, int numInsts, int &leftbound, int &rightbound){
	clock_t ta=clock();
	for(int i=0; i<numInsts; i++){
		int InstsY2 = Insts[i].y;
		int InstsWidth2 = Insts[i].x2 - Insts[i].x1;
		for(int j=0; j<numRows; j++){
			if(InstsY2==Rows[j].y){
				Rows[j].surplusonRow+=InstsWidth2;
			}
		}
	}
	int rowWidth2 = rightbound - leftbound;
	for(int j=0; j<numRows; j++){
		Rows[j].surplusonRow-=rowWidth2;
	}
	//ta=clock()-ta;
	//printf ("time before juggling for loop: %.3f\n",ta,((float)ta)/CLOCKS_PER_SEC);
	//clock_t tb=clock();
	int overcap = 0;
	for(int i=0; i<numRows; i++){
		if(Rows[i].surplusonRow>0){
			overcap=1;
		}
	}
	InstVec InstOnRow2;
	for(int i=0; i<numInsts; i++){
		int InstsY = Insts[i].y;
		for(int j=0; j<numRows; j++){
			if(Rows[j].y==InstsY){
				Insts[i].surplus=Rows[j].surplusonRow;
			}
		}
	}
	for(int j=0 ; j < numInsts ; j++){
		if(Insts[j].surplus>0){
			InstOnRow2.insert(InstOnRow2.end(), &Insts[j]);
		}
	}

	int verticalPert = 0;
	int verticalMove = 0;
	//tb=clock()-tb;
	//printf ("initial calculation for juggling for loop: %.3f\n",tb,((float)tb)/CLOCKS_PER_SEC);
	while(overcap==1){
		int bestHPWL = 999999999;
		int bestRow=-1;
		int bestCell=-1;
		clock_t tc=clock();
		for(int j=0; j<InstOnRow2.size(); ++j){
			int cellWidth2 = InstOnRow2.at(j)->x2-InstOnRow2.at(j)->x1;
			int cellY = InstOnRow2.at(j)->y;
			for(int k=0; k<numRows; ++k){
				int Rowksurplus = Rows[k].surplusonRow;
				if((Rowksurplus<0)&&((-Rowksurplus)>=cellWidth2)){
					int deltaHPWL=abs(Rows[k].y-cellY);
					if(deltaHPWL<bestHPWL){
						bestHPWL=deltaHPWL;
						bestRow=k;
						bestCell=j;
					}
				}
			}
		}
		if(bestCell==-1){
			break;
		}
		else{
			int formerRow=-1;
			int bestCellY = InstOnRow2.at(bestCell)->y;
			for(int i=0; i<numRows; ++i){
				if(Rows[i].y==bestCellY){
					formerRow=i;
				}
			}
			InstOnRow2.at(bestCell)->SetCenterCoord(InstOnRow2.at(bestCell)->x,Rows[bestRow].y);
			//================update surplus of start and destination row========================
			int bestCellWidth = InstOnRow2.at(bestCell)->x2-InstOnRow2.at(bestCell)->x1;
			Rows[formerRow].surplusonRow-=bestCellWidth;
			Rows[bestRow].surplusonRow+=bestCellWidth;
			overcap=0;
			for(int i=0; i<numRows; ++i){
				if(Rows[i].surplusonRow>0){
					overcap=1;
				}
			}
			InstOnRow2.erase(InstOnRow2.begin()+bestCell);
			int formerRowY = Rows[formerRow].y;
			if(Rows[formerRow].surplusonRow<=0){
				int vector_size = InstOnRow2.size();
				for(int i=0;i<vector_size;++i){
					if(InstOnRow2.at(i)->y==formerRowY){
						InstOnRow2.erase(InstOnRow2.begin()+i);
						--i;
						--vector_size;
					}
				}
			}

		}
		//tc=clock()-tc;
		//printf ("time on juggling for loop: %.3f\n",tc,((float)tc)/CLOCKS_PER_SEC);
	}
	//cout<<"vertical perturbation: "<<verticalPert<<endl;
	//cout<<"vertical movements: "<<verticalMove<<endl;
	return Rows[0].surplusonRow;
}


typedef std::vector<MisalignInst*> InstVec;


void overlap_removal(oaBlock* block, MisalignRow* Rows, MisalignInst* Insts, int numRows, int numInsts, int max_surplus){
	qsort(Insts, numInsts, sizeof(MisalignInst),compareInst_x);
	for(int i=0 ; i < numRows ; i++){
		InstVec InstOnRow;
		int row_width = Rows[i].x2 - Rows[i].x1;
		int site_width, NumSites=Rows[i].savedRow->getNumSites();
		site_width = row_width / NumSites;
		int scale = NumSites * site_width;
		if(scale < 100000){
			NumSites *= (site_width/20);
			site_width=20;
		}
		int max = Rows[i].surplusonRow;
		for(int j=0 ; j < numInsts ; j++){
			if(Insts[j].y == Rows[i].y){
				Insts[j].SetSite(int((Insts[j].x1-Rows[i].x1)/site_width));
				InstOnRow.insert(InstOnRow.end(), &Insts[j]);
			}
		}
		if(max>0){
			NumSites += (max/site_width);
		}
		int cell_number = InstOnRow.size();
		NumSites+=1;
		int* table = (int*) malloc(sizeof(int)*((cell_number+1)*NumSites));
		for(int p=0 ; p<cell_number+1 ; p++){
			table[p*NumSites] = 999999999;
		}
		for(int k=0 ; k<NumSites ; k++){
			table[k] = 0;
		}
		for(int p=1 ; p<cell_number+1 ; p++){
			for(int k=1 ; k<NumSites ; k++){
				int cell_width = (InstOnRow.at(p-1)->x2 - InstOnRow.at(p-1)->x1)/site_width;
				int x_cost = abs(InstOnRow.at(p-1)->site - (k-cell_width));
				if(k >= cell_width){
					int min = table[(p-1)*NumSites+k-cell_width] + x_cost;
					if(table[p*NumSites+k-1]<min){min = table[p*NumSites+k-1];}
					table[p*NumSites+k] = min;
				}
				else{
					table[p*NumSites+k] = table[p*NumSites+k-1];
				}
			}
		}
		// Traverse Backwards and Replace
		int k=NumSites-1;
		for(int p=cell_number ; p>0 ; p--){
			int cell_width = (InstOnRow.at(p-1)->x2 - InstOnRow.at(p-1)->x1)/site_width;
			int same_cell = 1;
			while(k>0 && same_cell ==1){
				int min = table[p*NumSites+k-1];
				int x_cost = abs(InstOnRow.at(p-1)->site - (k-cell_width));
				if( (x_cost + table[(p-1)*NumSites+k-cell_width])  <  min){ // Go up
					k-=cell_width;
					InstOnRow.at(p-1)->SetSite(k);
					same_cell = 0;
				}
				else{k--;}
			}
		}
		for(int j=0 ; j<InstOnRow.size() ; j++){
			int c_width = InstOnRow.at(j)->x2 - InstOnRow.at(j)->x1;
			InstOnRow.at(j)->x1 = Rows[i].x1 + (InstOnRow.at(j)->site*site_width);
			InstOnRow.at(j)->x2 = InstOnRow.at(j)->x1 + c_width;
			InstOnRow.at(j)->x = (InstOnRow.at(j)->x1 + InstOnRow.at(j)->x2)/2;
		}
		free(table);
	}

}

// ****************************************************************************
// main()
//
// This is the top level function that opens the design, prints library, cell,
// and view names, creates nets, and iterates the design to print the net 
// names.
// ****************************************************************************
int main(int    argc,	char   *argv[])
{
	try {
		// Initialize OA with data model 3, since incremental technology
		// databases are supported by this application.
		oaDesignInit(oacAPIMajorRevNumber, oacAPIMinorRevNumber, 3);
		oaString                librarypath="./";
		librarypath+=argv[1];
		oaString                libPath(librarypath);
		oaString                library(argv[1]);
		oaViewType      	*viewType = oaViewType::get(oacMaskLayout);
		oaString        	cell(argv[2]);
		oaString        	view("layout"); 
		oaScalarName            libName(ns,library);
		oaScalarName            cellName(ns,cell);
		oaScalarName            viewName(ns,view);
		oaScalarName    	libraryName(ns,library);
		// Setup an instance of the oaTech conflict observer.
		opnTechConflictObserver myTechConflictObserver(1);
		// Setup an instance of the oaLibDefList observer.
		opnLibDefListObserver   myLibDefListObserver(1);
		// Read in the lib.defs file.
		oaLib *lib = oaLib::find(libraryName);
		if (!lib) {
			if (oaLib::exists(libPath)) {
				// Library does exist at this path but was not in lib.defs
				lib = oaLib::open(libraryName, libPath);
			} else {
				char *DMSystem=getenv("DMSystem");
				if(DMSystem){
					lib = oaLib::create(libraryName, libPath, oacSharedLibMode, DMSystem);
				} else {
					lib = oaLib::create(libraryName, libPath);
				}
			}
			if (lib) {
				// We need to update the user's lib.def file since we either
				// found or created the library without a lib.defs reference.
				updateLibDefsFile(libraryName, libPath);
			} else {
				// Print error mesage 
				cerr << "ERROR : Unable to create " << libPath << "/";
				cerr << library << endl;
				return(1);
			}
		}
		// Create the design with the specified viewType,
		// Opening it for a 'write' operation.
		oaDesign    *design = oaDesign::open(libraryName, cellName, viewName, viewType, 'a');

		// Get the TopBlock for this design.
		oaBlock *block = design->getTopBlock();

		// If no TopBlock exist yet then create one.
		if (!block) {
			block = oaBlock::create(design);
		}

		//Rows and Insts Initialization************************************************/
		oaCollection<oaRow,oaBlock> CollectionRows = block->getRows();
		int numRows = CollectionRows.getCount();
		MisalignRow* Rows = (MisalignRow*) malloc(sizeof(MisalignRow)*numRows);
		oaCollection<oaInst,oaBlock> CollectionInsts=block->getInsts();
		int numInsts = CollectionInsts.getCount();
		cout<<"num Insts: "<<numInsts<<endl;
		MisalignInst* Insts = (MisalignInst*) malloc(sizeof(MisalignInst)*numInsts);
		/******************************************************************************/
		int leftbound;
		int rightbound;
		//misalignment removal starts here*********************************************/
		//clock_t t1=clock();
		misalignment_removal(block,Rows, Insts, numRows, numInsts, leftbound, rightbound);
		cout<<"misalignment removal completed!  " << endl;
		//t1=clock()-t1;
		//printf ("Taking %.3f seconds.\n",t1,((float)t1)/CLOCKS_PER_SEC);

		/******************************************************************************/



		//meet row capacity constraints************************************************/
		//clock_t t2=clock();
		int max_surplus = meet_row_capacity_constraints(block ,Rows ,Insts ,numRows, numInsts, leftbound, rightbound);
		cout<<"capacity optimization compledted! " << endl;
		//t2=clock()-t2;
		//printf ("Taking %.3f seconds.\n",t2,((float)t2)/CLOCKS_PER_SEC);
		/******************************************************************************/


		//overlap removal starts here**************************************************/
		//clock_t t3=clock();
		overlap_removal(block ,Rows ,Insts ,numRows, numInsts, max_surplus);
		cout<<"overlap removal completed! " << endl;
		//t3=clock()-t3;
		//printf ("Taking %.3f seconds.\n",t3,((float)t3)/CLOCKS_PER_SEC);
		/******************************************************************************/



		/*Write back the placement results to OA database*/
		for(int index=0 ; index < numInsts ; index++){
			Insts[index].Place();
		}


		design->save();
		// The design is closed.   
		design->close();

		// The library is closed.   
		lib->close();

	} catch (oaCompatibilityError &ex) {
		handleFBCError(ex);
		exit(1);

	} catch (oaException &excp) {
		cout << "ERROR: " << excp.getMsg() << endl;
		exit(1);
	}

	return 0;
}
