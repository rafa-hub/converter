 /*  Multi-neighborhood tabu search for the maximum weight clique problem
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This program demonstrates the use of the heuristic algorithm for solving
 * the problem of the maximum weight clique problem. For more information about this algorithm, 
 * please visit the website http://www.info.univ-angers.fr/pub/hao/mnts_clique.html or email to: wu@info-univ.angers.fr.
 */             

#include "mnts.h"

using namespace std;

// section 0, initiaze

mnts::~mnts(){

    delete[] vectex;
    delete[] funch;
    delete[] address;
    delete[] tabuin;
    delete[] adjaclen;
    delete[] C0;
    delete[] C1;
    delete[] TC1;
    delete[] We;
    delete[] BC;
    delete[] FC1;
    delete[] cruset;
    delete[] Tbest;
    delete[] TTbest;


    for (int x = 0 ; x < Max_Vtx ; x++ )
    {
    	delete[] Edge[x];
    	delete[] adjacMatrix[x];
    }
    delete[] Edge;
    delete[] adjacMatrix;

}
int mnts::randomInt( int n )
{
    return rand() % n;
}

void mnts::clearGamma()
{
    int i, j, k, l;
	
	tm1 = Max_Vtx*sizeof( int );

    memset( vectex, 0, tm1 );
    memset(  funch, 0, tm1 );
    memset(address, 0, tm1 );
    memset( tabuin, 0, tm1 );
    for( i = 0; i < Max_Vtx; i++ )
    {
       C0[ i ] = i;
       address[ i ] = i;
    }
    len0 = Max_Vtx;
    len1 = 0;
    len = 0;
    Wf = 0;
    Wbest = 0;
}

int mnts::selectC0(int Iter )
{
    int i, j, k, l, m;
    l = 0;
    if( len0 > 30 )
    {
       k = randomInt( len0 );
       return k;
    }
    for( i = 0; i < len0; i++ )
    {
       k = C0[ i ];
       if( tabuin[ k ] <= Iter )
         TC1[ l++ ] = i;
    }
    if( l == 0 )
      return -1;
    else
    {
        k = randomInt( l );
        k = TC1[ k ];
        return k;
    }
}

int mnts::WselectC0( int Iter)
{
    int i, j, k, l1, l2, w1, w2, m;
    l1 = 0;
    l2 = 0;
    w1 = 0;
    w2 = 0;
    
    for( i = 0; i < len0; i++ )
    {
       k = C0[ i ];
       if( tabuin[ k ] <= Iter )
       {
           if( We[ k ] > w1 )
           {
              l1 = 0;
              w1 = We[ k ];
              FC1[ l1++ ] = i;
           }
           else if ( We[ k ] >= w1 )
           {
              FC1[ l1++ ] = i;
           }
       }
       else
       {
           if( We[ k ] > w2 )
           {
              l2 = 0;
              w2 = We[ k ];
              TC1[ l2++ ] = i;
           }
           else if ( We[ k ] >= w2 )
           {
              TC1[ l2++ ] = i;
           }
       }
    }
    
    if( (l2 > 0) && ( w2 > w1 ) && ((w2+Wf)>Wbest) )
    {
        k = randomInt( l2 );
        k = TC1[ k ];
        //cout << "yes in aspiration w2+Wf = " << w2+Wf << endl;
        //getchar();
        return k;
    }  
    else if( l1 > 0 )
    {
        k = randomInt( l1 );
        k = FC1[ k ];
        return k;
    }
    else
    {
        return -1;
    }
}

int mnts::expand(int SelN)
{
    int i, j, k, k1, l, am, m, n, n1;
    
    m = C0[ SelN ];
    cruset[ len++ ] = m;
    vectex[ m ] = 1;
    Wf = Wf + We[ m ];
    
    len0--;
    n1 = C0[ len0 ];
    k1 = address[ m ];
    C0[ k1 ] = n1;
    address[ n1 ] = k1;
    
    for( i = 0; i < adjaclen[ m ]; i++ )
    {
       n = adjacMatrix[ m ][ i ];
       funch[ n ]++;
       if( funch[ n ] == 1 )
       {
           k1 = address[ n ];
           len0--;
           n1 = C0[ len0 ];
           C0[ k1 ] = n1;
           address[ n1 ] = k1;
           
           C1[ len1 ] = n;
           address[ n ] = len1;
           len1++;
           BC[ n ] = m;
       }
       else if( funch[ n ] == 2 )
       {
           len1--;
           n1 = C1[ len1 ];
           k1 = address[ n ];
           C1[ k1 ] = n1;
           address[ n1 ] = k1;
       }
    } 
    
    if( Wf > Wbest )
     {
        Wbest = Wf;
        len_best = len;
        for( i = 0; i < Max_Vtx; i++ )		//remove if there is no need for a list of nodes (pss)
        {
            Tbest[ i ] = vectex[ i ];
        }
     }
    
    return 1;   
}

int mnts::selectC1( int Iter)
{
    int i, j, k, l, m;
    l = 0;
    for( i = 0; i < len1; i++ )
    {
       k = C1[ i ];
       if( tabuin[ k ] <= Iter )
         TC1[ l++ ] = i;
    }
    if( l == 0 )
      return -1;
    else
    {
        k = randomInt( l );
        k = TC1[ k ];
        return k;
    }
}

int mnts::WselectC1(int Iter )
{
     int i, j, k, l, l1, l2, wmn, w1, w2, m, n;
     l1 = 0;
     l2 = 0;
     w1 = -1000000;
     w2 = -1000000;
     l = 0;
     for( i = 0; i < len1; i++ )
     {
         m = C1[ i ];
         n = BC[ m ];
         if( (vectex[ n ] == 1) && (Edge[ m ][ n ] == 1) )
           l++;
         else
         {
             for( j = 0; j < len; j++ )
             {
                k = cruset[ j ];
                if( Edge[ m ][ k ] == 1 )
                  break;
             }
             BC[ m ] = k;
         }
     }
     //cout << "len1 = " << len1 << " l = " << l << endl;
     for( i = 0; i < len1; i++ )
     {
         m = C1[ i ];
         n = BC[ m ];
         wmn = We[ m ] - We[ n ];
         if( tabuin[ m ] <= Iter )
         {
             if( wmn > w1 )
             {
                l1 = 0;
                w1 = wmn;
                FC1[ l1++ ] = i;
             }
             else if ( wmn >= w1 )
             {
                FC1[ l1++ ] = i;
             }
         }
         else
         {
             if( wmn > w2 )
             {
                l2 = 0;
                w2 = wmn;
                TC1[ l2++ ] = i;
             }
             else if ( wmn >= w2 )
             {
                TC1[ l2++ ] = i;
             }
         }
     }
     
     if( (l2 > 0) && ( w2 > w1 ) && ((w2+Wf)>Wbest) )
     {
         k = randomInt( l2 );
         k = TC1[ k ];
         return k;
     }  
     else if( l1 > 0 )
     {
         k = randomInt( l1 );
         k = FC1[ k ];
         return k;
     }
     else
     {
         return -1;
     }
}

int mnts::plateau( int SelN, int Iter )
{
     int i, j, k, k1, l, m0, m, m1, n, n1, mm1, ti;
     
     m = C1[ SelN  ];
     for(ti = 0; ti < len; ti++)
     {
         m1 = cruset[ ti ];
         if( Edge[ m1 ][ m ] == 1 )
            break;
     }
     
     Wf = Wf + We[ m ] - We[ m1 ];
     
     //the expand process, put m into the current independent set
     vectex[ m ] = 1;
     cruset[ len++ ] = m;
     //delete m from C1
     k1 = address[ m ];
     len1--;
     n1 = C1[ len1 ];
     C1[ k1 ] = n1;
     address[ n1 ] = k1;
     
     for( i = 0; i < adjaclen[ m ]; i++ )
     {
        n = adjacMatrix[ m ][ i ];
        funch[ n ]++;
        if( (funch[ n ] == 1) && ( vectex[ n ] == 0 ) )
        {
             //cout << "tt k1 = " << k1 << "len0 = " << len0 << "n = " << n << "m = " << m << " m1 = " << m1 << endl;
             k1 = address[ n ];
             len0--;
             n1 = C0[ len0 ];
             C0[ k1 ] = n1;
             address[ n1 ] = k1;
             
             C1[ len1 ] = n;
             address[ n ] = len1;
             len1++;
             BC[ n ] = m;
           
             //getchar();
        }
        if( funch[ n ] == 2 )
        {
            len1--;
            n1 = C1[ len1 ];
            k1 = address[ n ];
            C1[ k1 ] = n1;
            address[ n1 ] = k1;
        }        
     } 
     
     //the backtrack process, delete m1 from the current independent set
     vectex[ m1 ] = 0;
     //cout << "len1 = " << len1 << endl;
     tabuin[ m1 ] = Iter + TABUL + randomInt( len1+2 );
     len--;
     cruset[ ti ] = cruset[ len ];
     C1[ len1 ] = m1;
     address[ m1 ] = len1;
     len1++;
     
     for( i = 0; i < adjaclen[ m1 ]; i++ )
     {
        n = adjacMatrix[ m1 ][ i ];
        funch[ n ]--;
        if( (funch[ n ] == 0) && (vectex[ n ] == 0) )
        {
           k1 = address[ n ];           
           len1--;
           n1 = C1[ len1 ];
           C1[ k1 ] = n1;
           address[ n1 ] = k1;
           
           C0[ len0 ] = n;
           address[ n ] = len0;
           len0++;
        }
        else if( funch[ n ] == 1 )
        {
           C1[ len1 ] = n;
           address[ n ] = len1;
           len1++;
        }
     }
     
     if( Wf > Wbest )
     {
        Wbest = Wf;
        len_best = len;
        for( i = 0; i < Max_Vtx; i++ ) {			//remove if there is no need for a list of nodes (pss)
            Tbest[ i ] = vectex[ i ];
        }
     }
     return 1;   
}


int mnts::Mumi_Weigt()
{
    int i, j, k, l1, m;
    int w1 = 5000000;
    l1 = 0;
    for( i = 0; i < len; i++ )
    {
       k = cruset[ i ];
       if( We[ k ] < w1 )
       {
          l1 = 0;
          w1 = We[ k ];
          FC1[ l1++ ] = i;
       }
       else if ( We[ k ] <= w1 )
       {
          FC1[ l1++ ] = i;
       }
    }
    
    if( l1 == 0 )
      return -1;
    k = randomInt( l1 );
    k = FC1[ k ];
    return k;
}

int mnts::backtract(int Iter)
{
     int i, j, k, l, m, m1, n, ti, k1, n1;
     ti = Mumi_Weigt();
     if( ti == -1 )
      return -1;
     m1 = cruset[ ti ];
     Wf = Wf - We[ m1 ];
     vectex[ m1 ] = 0;
     tabuin[ m1 ] = Iter + TABUL;
     len--;
     cruset[ ti ] = cruset[ len ];
     C0[ len0 ] = m1;
     address[ m1 ] = len0;
     len0++;
     
     for( i = 0; i < adjaclen[ m1 ]; i++ )
     {
        n = adjacMatrix[ m1 ][ i ];
        funch[ n ]--;
        if( (funch[ n ] == 0) && (vectex[ n ] == 0) )
        {
           k1 = address[ n ];           
           len1--;
           n1 = C1[ len1 ];
           C1[ k1 ] = n1;
           address[ n1 ] = k1;
           
           C0[ len0 ] = n;
           address[ n ] = len0;
           len0++;
        }
        else if( funch[ n ] == 1 )
        {
           C1[ len1 ] = n;
           address[ n ] = len1;
           len1++;
        }
     }
}

int tabu( int Max_Iter, mnts &m )
{
     int i, j, k, l, bestlen = 0, am, am1, ww, ww1, ww2, ti, m1;
     int Iter = 0;
     m.clearGamma();
     while( 1 )
     {
        am = m.selectC0(Iter);
        if( am != -1 )
        {
            l = m.expand( am );
            Iter++;
            //if( Wbest == Waim )
            //   return Wbest;
        }
        else 
            break;
     }
      
     while( Iter < Max_Iter )
     {
        am = m.WselectC0(Iter);
        am1 = m.WselectC1(Iter);
        if( (am != -1) && (am1 != -1) )
        {
            ww = m.We[ m.C0[ am ] ];
            ww1 = m.We[ m.C1[ am1 ] ] - m.We[ m.BC[ m.C1[ am1 ] ] ];
        
            if( ww > ww1 )
            {
                l = m.expand( am );
                
                Iter++;
                //if( Wbest == Waim )
                //   return Wbest;
            }
            else
            {
                l = m.plateau( am1, Iter );
                //if( Wbest == Waim )
                //    return Wbest; 
                Iter++;
            }
        }
        else if( (am != -1) && (am1 == -1) )
        {
             l = m.expand( am );
             //if( Wbest == Waim )
             //  return Wbest;
                
             Iter++;
        }
        else if( (am == -1) && (am1 != -1) )
        {
             ti = m.Mumi_Weigt();
             m1 = m.cruset[ ti ];
             ww1 = m.We[ m.C1[ am1 ] ] - m.We[ m.BC[ m.C1[ am1 ] ] ];
             ww2 = - m.We[ m1 ];
             if( ww1 > ww2 )
             {
                l = m.plateau( am1,Iter );
                if( m.Wbest == m.Waim )
                    return m.Wbest;
                Iter++;
             }
             else
             {
                 k = m.backtract(Iter);
                 if( k == -1 )
                     return m.Wbest;
                 Iter++;
             }
        }
        else if( (am == -1) && (am1 == -1) )
        {
             k = m.backtract(Iter);
             if( k == -1 )
                return m.Wbest;
             Iter++;
        }
             
     }
     //cout<<"max iter reached wbest:"<<Wbest<< " len_best:" << len_best<<endl;
     return m.Wbest;
}

void mnts::verify()
{
     int i, j, k1, k2, l, m;
     for( i = 0; i < Max_Vtx; i++ )
     {
          if( TTbest[ i ] == 1 )
          {
              for( j = i+1; j < Max_Vtx; j++ )
              if( (TTbest[ j ] == 1) && ( Edge[ i ][ j ] == 1 ) )
                  cout << "hello there is something wrong" << endl;
          }
     }
}

/*void mnts::Output()
{
    int i , j, k, l, sum; 
    FILE *fp ;
    int len = strlen(File_Name);
    strcpy(outfilename,File_Name) ;
    outfilename[len]='.';
    outfilename[len+1]='o';
    outfilename[len+2]='u';
    outfilename[len+3]='t';
    outfilename[len+4]='\0';

    fp = fopen(outfilename, "a+"); 
    for( i = 0; i < 100; i++ )
    {
        fprintf(fp, "sum = %6d, iter = %6d, len = %5d,  time = %8lf \n", W_used[ i ], Iteration[ i ], len_used[ i ],  time_used[ i ] ); 
    }
    
    fprintf(fp, "\n\n the total information: \n");
    int wavg, iteravg, lenbb, success;
    wavg = iteravg = lenbb = success = 0;
    int best_v = 0;
    double timeavg = 0; 
    for( i = 0; i < 100; i++ )
    if( W_used[ i ] > best_v )
    {
        best_v = W_used[ i ];  
        lenbb = len_used[ i ];
    }
    
    int count = 0;
    fprintf(fp, "\n The best weight value for the maximum weighted problem is %6d \n", best_v);
    for( i = 0; i < 100; i++ )
    {
       wavg = wavg + W_used[ i ];
    }  
    double twavg = (double (wavg)) / 100 ; 
    for( i = 0; i < 100; i++ )
    if( W_used[ i ] == best_v )
    {
        count++;
        iteravg = iteravg + Iteration[ i ];
        timeavg = timeavg + time_used[ i ];
    }
    
    iteravg =  int ( (double (iteravg)) / count );
    timeavg = timeavg / (count*1000);
    fprintf(fp, "avg_sum = %10lf, succes = %6d, len = %5d, avg_iter = %6d,  time = %8lf \n", twavg, count, lenbb,  iteravg, timeavg );
    fclose(fp) ;
    return ;
}*/

/*
int main(int argc, char **argv)
{
       if ( argc == 5 )  
       {
          File_Name = argv[1];
          Waim = atoi(argv[2]);
          Wmode = atoi(argv[3]);
          len_improve = atoi(argv[4]);
       }  
       else 
       {
           cout << "Error : the user should input four parameters to run the program." << endl;
           //cout << "The input file_name is " << File_Name << endl;
           //cout << "The objective weight value is " << Waim << endl;
           //cout << "The value for Waim related to the way of allocating weights to vertices is " << Wmode << endl;
           //cout << "The search depth value is " << len_improve << endl;
           exit(0);
       }
       srand( (unsigned) time( NULL ) );
       Initializing();
       tm1 = Max_Vtx*sizeof( int );
       cout << "finish reading data" << endl;
       int i, j, k, l;
       len_time = (int (100000000 / len_improve) ) + 1;
       cout << "len_time = " << len_time << endl;
       for( i = 0; i < 100; i++ )
      {
          l = Max_Tabu();
          W_used[ i ] = l;
          len_used[ i ] = len_W;
          time_used[ i ] = finishing_time - starting_time;
          Iteration[ i ] = Titer;
          cout << "i = " << i << " l = " << l << endl;
      }
    
      Output();
      cout << "finished" << endl;
      getchar();
}
*/
