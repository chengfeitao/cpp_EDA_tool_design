#include <iostream>
#include "oaDesignDB.h"


using namespace oa; 
using namespace std;


class MisalignRow {
	public:
		oaInt4 x,y,x1,x2,y1,y2;
		int jugglingDone;
		oaRow* savedRow;
		int surplusonRow;
		int noInstonRow;

		//-------save a pointer to row and extract coordinates------------------
		void saveRow( oaRow* Row){
			savedRow = Row;
			oaBox vioBox;
			Row->getBBox(vioBox);
			oaPoint vioPoint;
			vioBox.getCenter(vioPoint);
			x = vioPoint.x();
			y = vioPoint.y();
			vioPoint= vioBox.lowerLeft();
			x1=vioPoint.x();
			y1=vioPoint.y();
			vioPoint= vioBox.upperRight();
			x2=vioPoint.x();
			y2=vioPoint.y();
			noInstonRow=0;
			jugglingDone=0;
			surplusonRow=0;
		}
};



