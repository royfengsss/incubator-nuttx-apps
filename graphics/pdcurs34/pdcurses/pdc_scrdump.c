/****************************************************************************
 * apps/graphics/pdcurs34/pdcurses/pdc_scrdump.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Adapted from the original public domain pdcurses by Gregory Nutt
 ****************************************************************************/

/* Name: scrdump
 *
 * Synopsis:
 *       int putwin(WINDOW *win, FILE *filep);
 *       WINDOW *getwin(FILE *filep);
 *       int scr_dump(const char *filename);
 *       int scr_init(const char *filename);
 *       int scr_restore(const char *filename);
 *       int scr_set(const char *filename);
 *
 * Description:
 *       getwin() reads window-related data previously stored in a file
 *       by putwin(). It then creates and initialises a new window using
 *       that data.
 *
 *       putwin() writes all data associated with a window into a file,
 *       using an unspecified format. This information can be retrieved
 *       later using getwin().
 *
 *       scr_dump() writes the current contents of the virtual screen to
 *       the file named by filename in an unspecified format.
 *
 *       scr_restore() function sets the virtual screen to the contents
 *       of the file named by filename, which must have been written
 *       using scr_dump(). The next refresh operation restores the screen
 *       to the way it looked in the dump file.
 *
 *       In PDCurses, scr_init() does nothing, and scr_set() is a synonym
 *       for scr_restore(). Also, scr_dump() and scr_restore() save and
 *       load from curscr. This differs from some other implementations,
 *       where scr_init() works with curscr, and scr_restore() works with
 *       newscr; but the effect should be the same. (PDCurses has no
 *       newscr.)
 *
 * Return Value:
 *       On successful completion, getwin() returns a pointer to the
 *       window it created. Otherwise, it returns a null pointer. Other
 *       functions return OK or ERR.
 *
 * Portability                                X/Open    BSD    SYS V
 *       putwin                                  Y
 *       getwin                                  Y
 *       scr_dump                                Y
 *       scr_init                                Y
 *       scr_restore                             Y
 *       scr_set                                 Y
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "curspriv.h"

/****************************************************************************
 * Pre-processor Defintiions
 ****************************************************************************/

#define DUMPVER 1               /* Should be updated whenever the WINDOW struct
                                 * is changed */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int putwin(WINDOW *win, FILE *filep)
{
  static const char *marker = "PDC";
  static const unsigned char version = DUMPVER;

  PDC_LOG(("putwin() - called\n"));

  /* Write the marker and the WINDOW struct */

  if (filep &&
      fwrite(marker, strlen(marker), 1, filep) &&
      fwrite(&version, 1, 1, filep) &&
      fwrite(win, sizeof(WINDOW), 1, filep))
    {
      int i;

      /* write each line */

      for (i = 0; i < win->_maxy && win->_y[i]; i++)
        {
          if (!fwrite(win->_y[i], win->_maxx * sizeof(chtype), 1, filep))
            {
              return ERR;
            }
        }

      return OK;
    }

  return ERR;
}

WINDOW *getwin(FILE *filep)
{
  WINDOW *win;
  char marker[4];
  int nlines;
  int ncols;
  int i;

  PDC_LOG(("getwin() - called\n"));

  if (!(win = malloc(sizeof(WINDOW))))
    {
      return NULL;
    }

  /* Check for the marker, and load the WINDOW struct */

  if (!filep || !fread(marker, 4, 1, filep) || strncmp(marker, "PDC", 3) ||
      marker[3] != DUMPVER || !fread(win, sizeof(WINDOW), 1, filep))
    {
      free(win);
      return NULL;
    }

  nlines = win->_maxy;
  ncols  = win->_maxx;

  /* Allocate the line pointer array */

  if (!(win->_y = malloc(nlines * sizeof(chtype *))))
    {
      free(win);
      return NULL;
    }

  /* allocate the minchng and maxchng arrays */

  if (!(win->_firstch = malloc(nlines * sizeof(int))))
    {
      free(win->_y);
      free(win);
      return NULL;
    }

  if (!(win->_lastch = malloc(nlines * sizeof(int))))
    {
      free(win->_firstch);
      free(win->_y);
      free(win);
      return NULL;
    }

  /* allocate the lines */

  if (!(win = PDC_makelines(win)))
    {
      return NULL;
    }

  /* read them */

  for (i = 0; i < nlines; i++)
    {
      if (!fread(win->_y[i], ncols * sizeof(chtype), 1, filep))
        {
          delwin(win);
          return NULL;
        }
    }

  touchwin(win);

  return win;
}

int scr_dump(const char *filename)
{
  FILE *filep;
#ifdef CONFIG_PDCURSES_MULTITHREAD
  FAR struct pdc_context_s *ctx = PDC_ctx();
#endif

  PDC_LOG(("scr_dump() - called: filename %s\n", filename));

  if (filename && (filep = fopen(filename, "wb")) != NULL)
    {
      int result = putwin(curscr, filep);
      fclose(filep);
      return result;
    }

  return ERR;
}

int scr_init(const char *filename)
{
  PDC_LOG(("scr_init() - called: filename %s\n", filename));

  return OK;
}

int scr_restore(const char *filename)
{
  FILE *filep;
#ifdef CONFIG_PDCURSES_MULTITHREAD
  FAR struct pdc_context_s *ctx = PDC_ctx();
#endif

  PDC_LOG(("scr_restore() - called: filename %s\n", filename));

  if (filename && (filep = fopen(filename, "rb")) != NULL)
    {
      WINDOW *replacement = getwin(filep);
      fclose(filep);

      if (replacement)
        {
          int result = overwrite(replacement, curscr);
          delwin(replacement);
          return result;
        }
    }

  return ERR;
}

int scr_set(const char *filename)
{
  PDC_LOG(("scr_set() - called: filename %s\n", filename));

  return scr_restore(filename);
}
