/* PLUGIN file to use with plantri.c 

   	To use this, compile plantri.c using for example
   	cc -o quad_restrict -O '-DPLUGIN="quad_restrict.c"' -DALLTOGETHER plantri.c

   	This plug-in deletes those 3-connected quadrangulations 
   	with vertex degrees which are not allowed. 
   	Allowed degrees may be defined by using the -F switch, for example

   	quad_restrict -F3F7F5 -q 14

   	makes all quadrangulations with 14 vertices and degrees 3, 5 or 7.

	The upper and lower limits for the number of faces to be used
	can be given like e.g. quad_restrict -F7_1^3F5F6 14 forcing between
	one and 3 vertices with valency 7. 
	This is implemented as a filter at the end. 

   	authors: Gunnar Brinkmann and Sebastian Funke

   	The nonstandard (but common) long long type is required.
   	(Warning: some versions of the Sun "cc" compiler give incorrect
   	results with programs using long long.)
*/

#include <math.h>
#include <ctype.h>

#ifdef UPPER_DEGREE_ERROR
#define PRE_FILTER_QUAD	calculate_the_upper_degree_error()
#endif

#ifdef DEGREE_ERROR
#define PRE_FILTER_QUAD calculate_the_degree_error()
#endif

#ifdef MAXIMAL_UPPER_DEGREE_ERROR
#define PRE_FILTER_QUAD calculate_the_maximal_upper_degree_error()
#endif

#ifdef SPECIAL_QUADS
#define PRE_FILTER_QUAD special_quads_pre_filter()
#endif

#ifdef ALLTOGETHER
#define PRE_FILTER_QUAD pre_filter_quad()

#define USE_LOCAL_METHOD_OF_RELATION
#define USE_GLOBAL_METHOD_OF_RELATION

#endif

#define FILTER ad_filter

/************************** Switches ********************************/

/* The following adds the switch f to those normally present.
   arg is the address of the command-line argument, and j is the
   index where 'F' might be.  The value of j must be left on the
   last digit of the switch value. */

#undef  SWITCHES
#define SWITCHES "[-F#[_#^#] -uagsh -odG -v -q]"

#define PLUGIN_SWITCHES else if (arg[j]=='F') list_of_allowed_degrees(arg, &j); 

/* if there are any other arguments than this ones that are allowed the program is terminated */
#define PLUGIN_INIT if (polygonsize >= 0 || minconnec >= 0\
   || minimumdeg >= 0 || pswitch || xswitch || tswitch || bswitch) \
  {fprintf(stderr,">E Usage: %s %s n [res/mod] [outfile]\n",cmdname,SWITCHES);\
   exit(1);}

/********************************************************************/

#define INFTY_AD (3*MAXN)

static unsigned long long int LIST = 0; 								/* a binary representation of the allowed degrees */
static unsigned long long int mask[MAXN];								/* ATTENTION: Here it is important to keep in mind that the number of bits of a long long Integer is equal to 64 = MAXN! */	

static int error_up[MAXN];												/* the i-th value in the list represents the up error of i */ 
static int error_down[MAXN];											/* the i-th value in the list represents the down error of i */
static int error_of_degree[MAXN];										/* the i-th value in the list represents the degree error of i */

static int up_or_down[MAXN];											/* the i-th value is 1 (0/-1) if up error of the vertex i is greater than (equal to/smaller than) the down error */
static int number_of_down_equal_up_errors = 0;							/* how many vertices has an up error that is equal to their down error */
static int number_of_down_errors = 0;									/* how many vertices has a smaller down than up error */
static int number_of_up_errors = 0;										/* how many vertices has a smaller up than down error */

static int maxnumber[MAXN],minnumber[MAXN];								/* the i-th value in the list represents how often the degree i must occur at least and how often he can occur at most */ 
static int bounds = 0;													/* TRUE if bounds are use otherwise FALSE */ 

static int buffer[MAXN];												/* used to sort the entries of allowed_degrees */
static int allowed_degrees[MAXN]; 										/* a list of allowed degrees ended by 0 */
static int number_of_allowed_degrees = 0;								/* counts the number of allowed degrees */

static int smallest_even_allowed_degree = 0;							/* saves the smallest even allowed degree */
static int maxdeg = 0; 													/* the maximum allowed degree */
static int maximal_distance_between_allowed_degrees = 0;				/* if the allowed degrees are in order this is the maximal distance between one allowed degree and the next */
static int all_allowed_degrees_are_odd = 1;								/* TRUE if there is no even degree otherwise FALSE */

static int special_case_for_mude = 0;									/* TRUE if ((maxnv % 2) && (maxdeg % 2) && (smallest_even_allowed_degree != 0)) else FALSE */

static int use_maximal_upper_degree_error = 0;							/* TRUE if S={3,x,x+1} mit x > 6 else FALSE */

static int no_quad_exist = 0;											/* TRUE if maxnv is odd and all allowed degrees are odd else FALSE */

static int b[MAXN];														/* used for calculate_the_maximal_upper_degree_error() */

static int x_param = 0;													/* the x of the definition of the degree error (x is allways equal 0.) */
static float mx = 0;													/* the quality how good the degree error can be reduced in each step (mx (is smaller or) equal 3)*/

static int number_of_rejections_a = 0;									/* some variables only for testing the program - they are not used */
static int number_of_rejections_b = 0;
static int mycounter = 0;

/********************************* Filter **************************/

/* The filter-function is used in the last step when there are only quadrangulations left that passed the prefilter-functions. */
/* At first the function checks if there is any quadrangulation left, that has vertex degrees that are not allowed.*/
/* Then it checks if there are to many or not enough vertices of a specific degree. */

static int ad_filter(int nbtot, int nbop, int doflip){
    int i,          /* counter from 0 to the number of vertices of the graph G */
	j,          /* counter from 0 to number_of_allowed_degrees */ 
	number_of_degree_x_in_G[MAXN];  /* counts how many vertices of the graph G are of the degree x */

    for(i = 0; i < nv; i++){
        if (error_of_degree[degree[i]]){
            return(0);
        }
    }
    
    if (bounds){
        /* Here it is important that the list of allowed degrees terminates with a null! */
        for (j = 0; allowed_degrees[j]; j++)
            /* The numbers of all the allowed degrees is initialized with null. */
            number_of_degree_x_in_G[allowed_degrees[j]] = 0;
			
    	for(i = 0; i < nv ; i++)
            /* increase number_of_degree_x_in_G[i] with one. */
            (number_of_degree_x_in_G[degree[i]])++;
			
    	for (j = 0; allowed_degrees[j]; j++)
            if ((number_of_degree_x_in_G[allowed_degrees[j]]<minnumber[allowed_degrees[j]]) ||
                    (number_of_degree_x_in_G[allowed_degrees[j]]>maxnumber[allowed_degrees[j]]))
                return(0);
  	}

    /* If the quadrangulation goes through the tests above it is accepted. */
    return(1);
}

static void check_for_bounding_arguments(char *arg, int *j, int *min_max){
    int k = (*j);

    if (arg[k+1] == '_'){
        bounds = bounds + 1;
        k+=2;
        *min_max = atoi(arg+k);
        while ((arg[k] >= '0') && (arg[k] <= '9'))
            k++;
        k--;
    } else {
        if (arg[k+1] == '^'){
            bounds = bounds + 2;
            k+=2;
            *min_max = atoi(arg+k);
            while ((arg[k] >= '0') && (arg[k] <= '9'))
                k++;
            k--;
        }
    }

    *j = k;		
}

/* The number k is out of the set S of allowed degrees */
/* The (not yet minimal) upper error of each degree between 3 (the minimal possible) and k is calculated. (k-i) */
/* If this upper error is smaller than before (for another k out of the set S) it is replaced. */
static void recalculate_error_up(int k){
    int i;
    
    for (i = 3; i <= k; i++)
        if ( error_up[i] > (k-i))
            error_up[i] = k-i;
}

/* The number k is out of the set S of allowed degrees */
/* The (not yet minimal) down error of each degree between k (greater 3) (the minimal possible) and MAXN is calculated. */
/* If this down error is smaller than before (for another k out of the set S) it is replaced. */
/* ATTENTION: The function makes the assumption that k is greater or equal to 3! (Another 'if' would cost to much.) */
static void recalculate_error_down(int k){
    int i;
    
    for(i = k; i < MAXN; i++)
        if (error_down[i] > (i-k))
            error_down[i] = i-k;
}

static void recalculate_error_of_degree(int x){
    int i;
    int a, b;
    
    for(i = 3; i < MAXN; i++){
        a = error_up[i];
        b = (1+x)*error_down[i];
        
        if (a < b){
            up_or_down[i] = 1;
        }
        if (a > b){
            up_or_down[i] = -1;
        }
        if (a == b){
            up_or_down[i] = 0;
        }
        
        error_of_degree[i] = MIN(error_up[i], (1+x)*error_down[i]);
    }
}

/************************** list_of_allowed_degrees ***********************/

/* list_of_allowed_degrees() analyse the command line arguments and calculates some values like:

 	The allowed degrees taken from the F-Switch LIST are described as bits 
   	of the binary representation of an integer.
   	The definition of the error_of_degree is explained in the text below. 
	
(1) all_allowed_degrees_are_odd 				= TRUE or FALSE
(2) maximal_distance_between_allowed_degrees    = maximal distance between one allowed degree and the next when they are in (decreasing) order
(3) maxdeg 										= biggest allowed degree
(4)	smallest_even_allowed_degree				= the smallest allowed degree that is even
(5) recalculate_error_up(n)						= calculates the up error of n
(6) recalculate_error_down(n)					= calculates the down error of n
(7)	recalculate_error_of_degree(n)				= calculates the degree error of n 

*/

static void list_of_allowed_degrees(char arg[], int *pj ){
    static int init = 1;
    
    int i, n, j;
    int distance;

    if (init){
    	init = 0;
    	mask[0] = 1;
    	error_up[0] = error_down[0] = error_of_degree[0] = INFTY_AD;
        allowed_degrees[0] = 0;
        buffer[0] = 0;
		
    	for(i = 1; i < MAXN; i++){
            mask[i] = mask[i-1]<<1;
            /* All upper, down and degree errors are initialized with the infinity. */
            error_up[i] = error_down[i] = error_of_degree[i] = INFTY_AD;
            /* At the beginning no degree is allowed. */
            allowed_degrees[i] = 0;
            buffer[i] = 0;
        }
    
    }

    j = *pj;

    if (!isdigit(arg[j+1])){
        fprintf(stderr,"No degrees given! Problem: %s\n",arg+j);
        exit(0);
    } else {
        /* get the switch values of option -F */
        j++;
        
        n = atoi (arg + j);
        
        if (n >= (MAXN-1)){
            fprintf(stderr,"Maximum n is %d!\n", MAXN-1);
            exit(0);
        }
        
        if (n < 3){
            fprintf(stderr,"Minimum n is 3!\n");
            exit(0);
        }
        
        LIST |= mask[n];
      	maxnumber[n] = MAXN;
        minnumber[n] = 0;
        
        /* the list allowed_degrees will be initialized for each allowed degree to get the right order */ 
        /* each allowed degree is listed only one time */
        /* the allowed degrees will be listed in decreasing order */
        number_of_allowed_degrees = 0;
        buffer[n] = 1;
        for (i = MAXN-1;i >= 3; i--)
            if (buffer[i])
                allowed_degrees[number_of_allowed_degrees++] = i;
        
        all_allowed_degrees_are_odd = 1;
        
        if (!(allowed_degrees[0]%2))
            all_allowed_degrees_are_odd = 0;
        
        maxdeg = allowed_degrees[0];
        
        maximal_distance_between_allowed_degrees = 0;
        for (i = 1; i < number_of_allowed_degrees; i++){
            distance = (allowed_degrees[i-1] - allowed_degrees[i]);
            if (distance > maximal_distance_between_allowed_degrees)
                maximal_distance_between_allowed_degrees = distance;
            /* if there is one even allowed degree not all degrees are odd */
            if (!(allowed_degrees[i]%2))
                all_allowed_degrees_are_odd = 0;
        }
        
        for(i = number_of_allowed_degrees-1; i >= 0; i--){
            if ((allowed_degrees[i]%2) == 0){
                smallest_even_allowed_degree = allowed_degrees[i];
                i = 0;
            }
        }
        
        if (number_of_allowed_degrees > 2){
            if (allowed_degrees[number_of_allowed_degrees-2] >= 6){
                use_maximal_upper_degree_error = 1;
                for (i = 0; i < number_of_allowed_degrees-2; i++){
                    if (allowed_degrees[i]-allowed_degrees[i+1] > 1)
                        use_maximal_upper_degree_error = 0;
                }
            }
        }
        
        recalculate_error_up(n);
        recalculate_error_down(n);
        recalculate_error_of_degree(x_param);
        
        while ((arg[j] >= '0') && (arg[j] <= '9')){
            j++;
        }
        j--;
        
        /* Two times the same to  be independent of the order */
        
        check_for_bounding_arguments(arg, &j, &minnumber[n]);
        check_for_bounding_arguments(arg, &j, &maxnumber[n]);
        
        if ((bounds != 3) && (bounds)){
            fprintf(stderr,"You must define both bounds, i.e. both the upper and the lower bound!\n");
            exit(0);
        }
        
        *pj = j;
    }
    mx = MAX(x_param + 3, 1);
}

/**************************************Some Auxiliary Functions **************************************/

static int calculate_faculty_of_n(int n){
    int i;
    int result = 1;
    
    for (i = 2; i <= n; i++)
        result *= i;
    
    return result;
}

static int power(int x, int y){
    int i;
    int result = 1;
    
    for (i = 0; i < y; i++)
        result *= x;
    
    return result;
}

/**************************************Pre Filter Quadrangulations************************************/

/************************************* UPPER DEGRREE ERROR METHOD *********************************/

static int calculate_the_upper_degree_error(){
    int i, upper_error_of_graph = 0;
    int difference = 0;
    
    for (i = 0; i < nv; i++){
        difference = (degree[i] - maxdeg);
        if (difference > 0)
            upper_error_of_graph += difference;
    }

/*
	if (upper_error_of_graph == (maxnv - nv))
	{
		for (i = 0; i < nv; i++)
		{
			if (degree[i] > maxdeg)
			{
				count = 0;
				run = last = firstedge[i];
				do
				{
					if (degree[run->end] >= maxdeg)
						count++; 

					run = run->next;
					
				} while (run != last);

				if (count == degree[i])
					return 0;
			}
		}
	}

*/
    return !(upper_error_of_graph > (maxnv - nv));	
}

/*********************************** METHODS OF RELATION ********************************************/

static int use_global_method_of_relation(){
    int i = 0;
    int j = 0;
    int fj = 0;
    int e_up = 0, e_down = 0;
    int up_equal_down[MAXN];
    int number_of_equals = 0;
    int sum = 0;
    int tmp;
    int j_exist = 0;

    for (i = 0; i < nv; i++){
        e_up = error_up[degree[i]];
        e_down = error_down[degree[i]];
        
        if (e_up < e_down)
            sum -= e_up;
        if (e_up > e_down)
            sum += 2*e_down;
        if (e_up == e_down){
            if (e_up)
                up_equal_down[j++] = e_up;
        }
    }
    
    number_of_equals = j;
    
    if (number_of_equals == 0)
        return sum;
    
    if (number_of_equals <= 3){
        fj = calculate_faculty_of_n(j);
        
        for (j = 0; j <= fj + 1; j++){
            tmp = sum;
            
            for (i = 0; i < number_of_equals; i++){
                if (power(2,i) & j)
                    sum -= up_equal_down[i];
                else
                    sum += 2 * up_equal_down[i];
            }
            
            if (!sum)
                j_exist = 1;
            
            sum = tmp;
        }
        
        if (!j_exist)
            return 1;
    }
    
    return 0;
}

static int use_local_method_of_relation(){
    int i = 0;
    int e_up = 0;
    int e_down = 0;
    int z = 0;
    
    int degree_of_neighbour = 0;
    int up_error_of_neighbour = 0;
    int down_error_of_neighbour = 0;
    
    int whole_error_of_neighbours = 0;
    
    EDGE *run;
    EDGE *start;
    EDGE *last;
    
    for (i = 0; i < nv; i++){
        start = firstedge[i];
        last = firstedge[i];
        
        e_up = error_up[degree[i]];
        e_down = error_down[degree[i]];
        
        z = e_up - e_down;
        
        if (z < 0){
            whole_error_of_neighbours = 0;
            run = start;
            do {
                degree_of_neighbour = degree[run->end];
                up_error_of_neighbour = error_up[degree_of_neighbour];
                down_error_of_neighbour = error_down[degree_of_neighbour];
                
                if (down_error_of_neighbour > 0){
                    if (down_error_of_neighbour <= up_error_of_neighbour){
                        whole_error_of_neighbours += down_error_of_neighbour;
                    }
                }
                
                run = run->next;
            
            } while (run != last);
            
            if (whole_error_of_neighbours < e_up){
                return 1;
            }
        }
        
        if (z > 0){
            whole_error_of_neighbours = 0;
            run = start;
            do {
                degree_of_neighbour = degree[run->end];
                up_error_of_neighbour = error_up[degree_of_neighbour];
                down_error_of_neighbour = error_down[degree_of_neighbour];
                
                if (up_error_of_neighbour > 0){
                    if (up_error_of_neighbour <= down_error_of_neighbour){
                        whole_error_of_neighbours += up_error_of_neighbour;
                    }
                }
                
                run = run->next;
            
            } while (run != last);
            
            if (whole_error_of_neighbours < 2*e_down){
                return 1;
            }
        
        }
        
        if (z == 0){
            whole_error_of_neighbours = 0;
            run = start;
            do {
                degree_of_neighbour = degree[run->end];
                up_error_of_neighbour = error_up[degree_of_neighbour];
                down_error_of_neighbour = error_down[degree_of_neighbour];
                
                if (up_error_of_neighbour > 0){
                    if (up_error_of_neighbour <= down_error_of_neighbour){
                        whole_error_of_neighbours += up_error_of_neighbour;
                    }
                }
                
                run = run->next;
            } while (run != last);
            
            if (whole_error_of_neighbours < 2*e_down){
                whole_error_of_neighbours = 0;
                run = start;
                do {
                    degree_of_neighbour = degree[run->end];
                    up_error_of_neighbour = error_up[degree_of_neighbour];
                    down_error_of_neighbour = error_down[degree_of_neighbour];
                    
                    if (down_error_of_neighbour > 0){
                        if (down_error_of_neighbour <= up_error_of_neighbour){
                            whole_error_of_neighbours += down_error_of_neighbour;
                        }
                    }
                    
                    run = run->next;
                } while (run != last);
                
                if (whole_error_of_neighbours < e_up) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*************************************DEGREE ERROR METHOD ***************************/

static int calculate_the_graph_error(){
    int i, error_of_graph = 0;
    
    for (i = 0; i < nv; i++){
        if (up_or_down[degree[i]]){
            if (up_or_down[degree[i]] > 0)
                number_of_up_errors += error_of_degree[degree[i]];
            else
                number_of_down_errors += error_of_degree[degree[i]];
        } else {
            number_of_down_equal_up_errors += error_of_degree[degree[i]];
        }
        error_of_graph += error_of_degree[degree[i]];
    }
    
    return error_of_graph;	 
}

static int calculate_the_degree_error(){
    int error_of_graph = calculate_the_graph_error();
    int number_of_remaining_steps = maxnv - nv;
    
    if (error_of_graph == (3 * number_of_remaining_steps)){
#ifdef USE_LOCAL_METHOD_OF_RELATION
        if (use_local_method_of_relation())
            return 0;	
#endif				
#ifdef USE_GLOBAL_METHOD_OF_RELATION
        if (use_global_method_of_relation())
            return 0;
#endif
        return 1;
    }
    
    return !(error_of_graph > (mx * number_of_remaining_steps));
}

/************** METHOD OF MAXIMAL UPPER DEGREE ERROR **********/

static int calculate_the_maximal_upper_degree_error(){
    int i = 0;
    int j = 0;
    int k = 0;
    int ordered_degrees[MAXN]; /* list of ordered degrees of the actual quadrangulation */
    int diff = 0;
    static int init = 1;
    int start = 0;
    int end = 0;
    int number_of_ops_for_down_correction = 0;
    
    int min_number_of_needed_operations = 0;
    
    special_case_for_mude = ((maxnv % 2) && (maxdeg % 2) && (smallest_even_allowed_degree != 0));

    /******************** BEGIN: some inits for calculate_the_maximal_upper_degree_error(). *****************************/
    
    if (init){
        for (i = 0; i < MAXN; i++)
            b[i] = 0;
        
        if (special_case_for_mude)
            b[1] = floor(((float)(maxnv-8-(smallest_even_allowed_degree-3)))/((float)(maxdeg-3)));
        else
            b[1] = floor(((float)(maxnv-8))/((float)(maxdeg-3)));
        
        for (k = 2; k < number_of_allowed_degrees; k++){
            b[k] = MIN(maxnv, floor(((float)(maxnv-8))/((float)(allowed_degrees[k-1]-3))));
        }
        b[number_of_allowed_degrees] = maxnv;
        
        init = 0;
    }
    
    /******************** END: some inits for calculate_the_maximal_upper_degree_error(). *****************************/
    
    for(i = 0; i < maxnv; i++){
        buffer[i] = 0;
        ordered_degrees[i] = 0;
    }
    
    for (i = 0; i < nv; i++)
        buffer[degree[i]]++;
    
    j = 0;
    for (i = maxnv-1; i > 0; i--)
        if (buffer[i] != 0){
            ordered_degrees[j++]=i;
            buffer[i]--;
            i++;
        }
    
    for (k = 0; k < number_of_allowed_degrees; k++){
        start = b[k];
        end = MIN(b[k+1],nv-8);
        
        for (i = start; i < end; i++){
            if (i < maxnv-8)
                diff = ordered_degrees[i] - allowed_degrees[k];
            else
                diff = ordered_degrees[i] - 3;
            
            if (diff > 0)
                number_of_ops_for_down_correction += diff;
        }
    }
    
    min_number_of_needed_operations = number_of_ops_for_down_correction;
    
    return !(min_number_of_needed_operations > (maxnv - nv));
}

/************* FILTER FOR QUADRANGULATIONS WITH TWO ALLOWED DEGREES ************/

static int a_x(int n){
    if (maxdeg == 4)
        return (n-8);
    else
        return (floor( ((float)(n-8)) / ((float)(maxdeg-3)) ));	
}

static int euler_test(){
    float test = (((float)(maxnv-8))/((float)(maxdeg-3)));
    
    if ((test - (int)test) == 0)
        return 1;
    else
        return 0;		
}

static int calculate_number_of_needed_operations(){
    int m;
    int sum = 0;
    int i = 0;
    int j = 0;
    int s_2 = 0;
    int ordered_degrees[MAXN]; /* list of ordered degrees of the actual quadrangulation */
    int can_use_better_cases = 1;
    int diff = 0;
    
    int min_number_of_needed_operations = 0;
    
    m = a_x(maxnv);
    
    for(i = 0; i < maxnv; i++){
        buffer[i] = 0;
        ordered_degrees[i] = 0;
    }
    
    for (i = 0; i < nv; i++)
        buffer[degree[i]]++;
    
    j = 0;
    for (i = maxnv-1; i > 0; i--)
        if (buffer[i] != 0){
            ordered_degrees[j++]=i;
            buffer[i]--;
            i++;
        }
    
    for (i = 0; i < m; i++){
        if (i < nv)
            diff = maxdeg - ordered_degrees[i];
        else
            diff = maxdeg - 3;
        
        sum += fabs(diff);
        if (diff < 0)
            can_use_better_cases = 0;
    }
    
    for (i = m; i < nv; i++){
        s_2 += (ordered_degrees[i] - 3);
    }
    
    if (2 * s_2 < sum){
        if (can_use_better_cases)
            min_number_of_needed_operations = ceil(0.5 * sum);
        else
            min_number_of_needed_operations = ceil((1.0/3.0)*(sum + s_2));
    } else
        min_number_of_needed_operations = s_2;
    
    return min_number_of_needed_operations;
}

static int special_quads_pre_filter(){
    int min_number_of_needed_operations;
    
    if (euler_test())
        min_number_of_needed_operations = calculate_number_of_needed_operations();
    else
        return 0;
    
    return !(min_number_of_needed_operations > (maxnv - nv));
}

/*************************************************** ALL METHODS TOGETHER *****************************************/

static int pre_filter_quad(){
    no_quad_exist = ((!smallest_even_allowed_degree) && (maxnv%2));
    
    if (no_quad_exist)
        return 0;
    else {
        if (maximal_distance_between_allowed_degrees == 1)
            return calculate_the_upper_degree_error();
        else {
            if (number_of_allowed_degrees == 2) {
                if (euler_test()) {
                    if (maxdeg == 5)
                        return calculate_the_degree_error();
                    else
                        return calculate_the_maximal_upper_degree_error();
                } else
                    return 0;
            } else {
                if (use_maximal_upper_degree_error)
                    return calculate_the_maximal_upper_degree_error();
                else
                    return calculate_the_degree_error();
            }
        }
    }
    
    return 1;
}
