/***
*mktime.c - Convert struct tm value to time_t value.
*
*       Copyright (c) 1987-1997, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines mktime() and _mkgmtime(), routines to converts a time value
*       in a tm structure (possibly incomplete) into a time_t value, then
*       update (all) the structure fields with "normalized" values.
*
*******************************************************************************/

#include "bms_time.h"

#pragma MESSAGE DISABLE C1825 //Indirection to different types
#pragma MESSAGE DISABLE C5909 // Assignment in condition
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

static const int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static const int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

/*
 * ChkAdd evaluates to TRUE if dest = src1 + src2 has overflowed
 */
#define ChkAdd(dest, src1, src2)   ( ((src1 >= 0L) && (src2 >= 0L) \
    && (dest < 0L)) || ((src1 < 0L) && (src2 < 0L) && (dest >= 0L)) )

/*
 * ChkMul evaluates to TRUE if dest = src1 * src2 has overflowed
 */
#define ChkMul(dest, src1, src2)   ( src1 ? (dest/src1 != src2) : 0 )

static struct tm tb = { 0 };    /* time block */

struct tm * date_time_from_time_t(const time_t *timp)
{

        long caltim = *timp;            /* calendar time to convert */
        int islpyr = 0;                 /* is-current-year-a-leap-year flag */
        int tmptim;
        int *mdays;                /* pointer to days or lpdays */

#ifdef _MT

        struct tm *ptb;            /* will point to gmtime buffer */
        _ptiddata ptd = _getptd();

#else  /* _MT */
        struct tm *ptb = &tb;
#endif  /* _MT */

        if (timp == NULL) return(NULL);
        
        if ( caltim < 0L )
                return(NULL);

#ifdef _MT

        /* Use per thread buffer area (malloc space, if necessary) */

        if ( (ptd->_gmtimebuf != NULL) || ((ptd->_gmtimebuf =
            _malloc_crt(sizeof(struct tm))) != NULL) )
                ptb = ptd->_gmtimebuf;
        else
                ptb = &tb;      /* malloc error: use static buffer */

#endif  /* _MT */

        /*
         * Determine years since 1970. First, identify the four-year interval
         * since this makes handling leap-years easy (note that 2000 IS a
         * leap year and 2100 is out-of-range).
         */
        tmptim = (int)(caltim / _FOUR_YEAR_SEC);
        caltim -= ((long)tmptim * _FOUR_YEAR_SEC);

        /*
         * Determine which year of the interval
         */
        tmptim = (tmptim * 4) + 70;         /* 1970, 1974, 1978,...,etc. */

        if ( caltim >= _YEAR_SEC ) {

            tmptim++;                       /* 1971, 1975, 1979,...,etc. */
            caltim -= _YEAR_SEC;

            if ( caltim >= _YEAR_SEC ) {

                tmptim++;                   /* 1972, 1976, 1980,...,etc. */
                caltim -= _YEAR_SEC;

                /*
                 * Note, it takes 366 days-worth of seconds to get past a leap
                 * year.
                 */
                if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) {

                        tmptim++;           /* 1973, 1977, 1981,...,etc. */
                        caltim -= (_YEAR_SEC + _DAY_SEC);
                }
                else {
                        /*
                         * In a leap year after all, set the flag.
                         */
                        islpyr++;
                }
            }
        }

        /*
         * tmptim now holds the value for tm_year. caltim now holds the
         * number of elapsed seconds since the beginning of that year.
         */
        ptb->tm_year = tmptim;

        /*
         * Determine days since January 1 (0 - 365). This is the tm_yday value.
         * Leave caltim with number of elapsed seconds in that day.
         */
        ptb->tm_yday = (int)(caltim / _DAY_SEC);
        caltim -= (long)(ptb->tm_yday) * _DAY_SEC;

        /*
         * Determine months since January (0 - 11) and day of month (1 - 31)
         */
        if ( islpyr )
            mdays = _lpdays;
        else
            mdays = _days;


        for ( tmptim = 1 ; mdays[tmptim] < ptb->tm_yday ; tmptim++ ) ;

        ptb->tm_mon = --tmptim;

        ptb->tm_mday = ptb->tm_yday - mdays[tmptim];

        /*
         * Determine days since Sunday (0 - 6)
         */
        ptb->tm_wday = ((int)(*timp / _DAY_SEC) + _BASE_DOW) % 7;

        /*
         *  Determine hours since midnight (0 - 23), minutes after the hour
         *  (0 - 59), and seconds after the minute (0 - 59).
         */
        ptb->tm_hour = (int)(caltim / 3600);
        caltim -= (long)ptb->tm_hour * 3600L;

        ptb->tm_min = (int)(caltim / 60);
        ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

        ptb->tm_isdst = 0;
        return( (struct tm *)ptb );

}

/***
*time_t mktime(tb) - Normalize user time block structure
*
*Purpose:
*       Mktime converts a time structure, passed in as an argument, into a
*       calendar time value in internal format (time_t). It also completes
*       and updates the fields the of the passed in structure with 'normalized'
*       values. There are three practical uses for this routine:
*
*       (1) Convert broken-down time to internal time format (time_t).
*       (2) To have mktime fill in the tm_wday, tm_yday, or tm_isdst fields.
*       (3) To pass in a time structure with 'out of range' values for some
*           fields and have mktime "normalize" them (e.g., pass in 1/35/87 and
*           get back 2/4/87).
*Entry:
*       struct tm *tb - pointer to a tm time structure to convert and
*                       normalize
*
*Exit:
*       If successful, mktime returns the specified calender time encoded as
*       a time_t value. Otherwise, (time_t)(-1) is returned to indicate an
*       error.
*
*Exceptions:
*       None.
*
*******************************************************************************/


time_t date_time_to_time_t(struct tm *tb)
{
    long tmptm1, tmptm2, tmptm3;
    struct tm *tbtemp;
    
    /*
     * First, make sure tm_year is reasonably close to being in range.
     */
    if(tb == NULL) goto err_mktime;
    
    tmptm1 = tb->tm_year;
    if ( (tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1) )
        goto err_mktime;


    /*
     * Adjust month value so it is in the range 0 - 11.  This is because
     * we don't know how many days are in months 12, 13, 14, etc.
     */

    if ( (tb->tm_mon < 0) || (tb->tm_mon > 11) ) {

        /*
         * no danger of overflow because the range check above.
         */
        tmptm1 += (tb->tm_mon / 12);

        if ( (tb->tm_mon %= 12) < 0 ) {
            tb->tm_mon += 12;
            tmptm1--;
        }

        /*
         * Make sure year count is still in range.
         */
        if ( (tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1) )
            goto err_mktime;
    }

    /***** HERE: tmptm1 holds number of elapsed years *****/

    /*
     * Calculate days elapsed minus one, in the given year, to the given
     * month. Check for leap year and adjust if necessary.
     */
    tmptm2 = _days[tb->tm_mon];
    if ( !(tmptm1 & 3) && (tb->tm_mon > 1) )
            tmptm2++;

    /*
     * Calculate elapsed days since base date (midnight, 1/1/70, UTC)
     *
     *
     * 365 days for each elapsed year since 1970, plus one more day for
     * each elapsed leap year. no danger of overflow because of the range
     * check (above) on tmptm1.
     */
    tmptm3 = (tmptm1 - _BASE_YEAR) * 365L + ((tmptm1 - 1L) >> 2)
      - _LEAP_YEAR_ADJUST;

    /*
     * elapsed days to current month (still no possible overflow)
     */
    tmptm3 += tmptm2;

    /*
     * elapsed days to current date. overflow is now possible.
     */
    tmptm1 = tmptm3 + (tmptm2 = (long)(tb->tm_mday));
    if ( ChkAdd(tmptm1, tmptm3, tmptm2) )
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed days *****/

    /*
     * Calculate elapsed hours since base date
     */
    tmptm2 = tmptm1 * 24L;
    if ( ChkMul(tmptm2, tmptm1, 24L) )
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_hour);
    if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed hours *****/

    /*
     * Calculate elapsed minutes since base date
     */

    tmptm2 = tmptm1 * 60L;
    if ( ChkMul(tmptm2, tmptm1, 60L) )
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_min);
    if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed minutes *****/

    /*
     * Calculate elapsed seconds since base date
     */

    tmptm2 = tmptm1 * 60L;
    if ( ChkMul(tmptm2, tmptm1, 60L) )
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_sec);
    if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed seconds *****/
    if ( (tbtemp = date_time_from_time_t(&tmptm1)) == NULL )
        goto err_mktime;
    
    *tb = *tbtemp;
    
    return (time_t)tmptm1;

    err_mktime:
    /*
     * All errors come to here
     */
    return (time_t)(-1);
}

void date_time_set_default(struct tm *tb)
{
    if(tb == NULL) return;
    
    tb->tm_year = _BASE_YEAR;
    tb->tm_mon = 0;
    tb->tm_mday = 1;
    tb->tm_hour = 0;
    tb->tm_min = 0;
    tb->tm_sec = 0;
    tb->tm_gmtoff = 0;
    tb->tm_isdst = 0;
    tb->tm_wday = _BASE_DOW;
    tb->tm_yday = 0;
}