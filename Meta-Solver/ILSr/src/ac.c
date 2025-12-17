/*    
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <math.h>
#include "utilities.h"
#include "ac.h"

static double Temperature;
static double InitialTemperature;
static int CoolingCDStart;
static double CoolingAlpha;


void InitSAAC(double alpha, int cd) {
    CoolingAlpha = alpha;
    CoolingCDStart = cd;
}

int AcceptanceCriterion(long long int l, long long int cb) {
    static int CoolingContDown;
    static int accepted=0;
    static int tried=0;
    static float tot;
    int r;

    if (!CoolingContDown) CoolingContDown = CoolingCDStart;

    switch (AcceptanceCriterionType) {
	case 'b':
	    return(FALSE);
	case 'r':
	    return(TRUE);
	case 'p':
	    return(l > cb*(1-ACParameter));
	case 's':
	    CoolingContDown--;
	    if (Temperature < 0) {
		/* Remember data for initializing Temperature */
		if (!CoolingContDown) {
		    Temperature = InitialTemperature = tot/(float)CoolingCDStart;
		    /* Initialize Temperature */
		}
		tot += ((l-cb)>0)?(l-cb):(cb-l);
		return(FALSE);
	    }
	    r=(ran01(&Seed)<exp((l-cb)/Temperature));
	    if (r) accepted++;
	    tried++;
	    if (!CoolingContDown) {
		CoolingContDown = CoolingCDStart;
		Temperature *= CoolingAlpha;
		if ( ( ((float)accepted) / ((float)tried) ) < 0.02 )
		    Temperature=InitialTemperature;
		accepted=0;
		tried=0;
	    }

	    return(r);
    }
    return(FALSE);
}
