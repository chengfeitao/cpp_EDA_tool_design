#include <iostream>
#include "oaDesignDB.h"


using namespace oa; 
using namespace std;


class MisalignInst {
	public:
		oaInt4 x,y,x1,x2,y1,y2,origx,origy,site, cx, cy;
		oaInst* savedInst;
		int row;
		int surplus;
		int errortype;
		oaSimpleName name;
		void SetSite(int X){
			site = X;
		}
		void MoveRight(int X){
			x += X;	
			x1 += X;	
			x2 += X;	
		}
		void MoveUp(int Y){
			y += Y;	
			y1 += Y;	
			y2 += Y;
		}
		void SetCenterCoord(int X, int Y){
			x = X;
			y = Y;
		}
		void SetLeftCoord(int X, int Y){
			x1 = X;
			y1 = Y;
		}
		void SetRightCoord(int X, int Y){
			x2 = X;
			y2 = Y;
		}
		void Place(){
			oaPoint CenterPoint;
			origx+=(x-cx);
			origy+=(y-cy);
			CenterPoint.set(origx,origy);
			savedInst->setOrigin(CenterPoint);
		}

		//-------save a pointer to inst and extract coordinates------------------
		void saveInst( oaInst* Inst){
			savedInst = Inst;
			oaBox vioBox;
			Inst->getBBox(vioBox);
			oaPoint vioPoint;
			vioBox.getCenter(vioPoint);
			x = vioPoint.x();
			y = vioPoint.y();
			cx=x;
			cy=y;
			vioPoint= vioBox.lowerLeft();
			x1=vioPoint.x();
			y1=vioPoint.y();
			vioPoint= vioBox.upperRight();
			x2=vioPoint.x();
			y2=vioPoint.y();

			oaPoint instOrigin;
			Inst->getOrigin(instOrigin);
			origx=instOrigin.x();
			origy=instOrigin.y();
			Inst->getName(name);
			errortype=0;
		}
};
