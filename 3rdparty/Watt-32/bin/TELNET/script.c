/*
 * Written by Archie Cobbs <archie@whistle.com>
 * Copyright (c) 1995, 1996 Whistle Communications, Inc. All rights reserved.
 *
 * Subject to the following obligations and disclaimer of warranty,
 * use and redistribution of this software, in source or object code
 * forms, with or without modifications are expressly permitted by
 * Whistle Communications; provided, however, that:   (i) any and
 * all reproductions of the source or object code must include the
 * copyright notice above and the following disclaimer of warranties;
 * and (ii) no rights are granted, in any manner or form, to use
 * Whistle Communications, Inc. trademarks, including the mark "WHISTLE
 * COMMUNICATIONS" on advertising, endorsements, or otherwise except
 * as such appears in the above copyright notice or in the software.
 *
 * THIS SOFTWARE IS BEING PROVIDED BY WHISTLE COMMUNICATIONS "AS IS",
 * AND TO THE MAXIMUM EXTENT PERMITTED BY LAW, WHISTLE COMMUNICATIONS
 * MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED,
 * REGARDING THIS SOFTWARE, INCLUDING WITHOUT LIMITATION, ANY AND
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT.  WHISTLE COMMUNICATIONS DOES NOT
 * WARRANT, GUARANTEE, OR MAKE ANY REPRESENTATIONS REGARDING THE USE
 * OF, OR THE RESULTS OF THE USE OF THIS SOFTWARE IN TERMS OF ITS
 * CORRECTNESS, ACCURACY, RELIABILITY OR OTHERWISE.  IN NO EVENT
 * SHALL WHISTLE COMMUNICATIONS BE LIABLE FOR ANY DAMAGES RESULTING
 * FROM OR ARISING OUT OF ANY USE OF THIS SOFTWARE, INCLUDING WITHOUT
 * LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * PUNITIVE, OR CONSEQUENTIAL DAMAGES, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES, LOSS OF USE, DATA OR PROFITS, HOWEVER CAUSED
 * AND UNDER ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF WHISTLE COMMUNICATIONS
 * IS ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <ctype.h>

#include "telnet.h"
#include "timer.h"
#include "config.h"
#include "script.h"

/* Keywords */

#define CHAT_VAR_PREFIX    '$'
#define CHAT_VAR_MATCHED   "$matchedString"

#define IF                 "if"
#define SET                "set"
#define MATCH              "match"
#define TIMER              "timer"
#define CANCEL             "cancel"
#define PRINT              "print"
#define SEND               "send"
#define CALL               "call"
#define LOG                "log"
#define RETURN             "return"
#define GOTO               "goto"
#define SUCCESS            "success"
#define FAILURE            "failure"
#define WAIT               "wait"
#define CHAT_KEYWORD_ALL   "all"

/* Special timer used when no label given */

#define DEFAULT_LABEL      ""
#define DEFAULT_SET        ""
#define BIG_LINE_SIZE      1000
#define TRUE               1
#define FALSE              0

static int lineNum;
static int chat_stop, chat_okay;

enum chat_states {
     CHAT_IDLE = 0,
     CHAT_RUNNING,
     CHAT_WAIT
   };

enum chat_commands {
     CMD_IF,
     CMD_SET,
     CMD_MATCH,
     CMD_TIMER,
     CMD_CANCEL,
     CMD_PRINT,
     CMD_SEND,
     CMD_CALL,
     CMD_LOG,
     CMD_RETURN,
     CMD_GOTO,
     CMD_SUCCESS,
     CMD_FAILURE,
     CMD_WAIT
   };

/*
 * INTERNAL VARIABLES
 */
static struct chatcmd {
       char *name;
       enum  chat_commands id;
       short min;
       short max;
     } commands[] = {
       { IF,      CMD_IF,      4, 0 },
       { SET,     CMD_SET,     3, 3 },
       { MATCH,   CMD_MATCH,   2, 4 },
       { TIMER,   CMD_TIMER,   2, 4 },
       { CANCEL,  CMD_CANCEL,  2, 0 },
       { PRINT,   CMD_PRINT,   2, 2 },
       { SEND,    CMD_SEND,    2, 2 },
       { CALL,    CMD_CALL,    2, 2 },
       { LOG,     CMD_LOG,     2, 2 },
       { RETURN,  CMD_RETURN,  1, 1 },
       { GOTO,    CMD_GOTO,    2, 2 },
       { SUCCESS, CMD_SUCCESS, 1, 1 },
       { FAILURE, CMD_FAILURE, 1, 1 },
       { WAIT,    CMD_WAIT,    1, 2 },
     };

#define CHAT_NUM_COMMANDS   (sizeof(commands)/sizeof(commands[0]))

/*
 * INTERNAL FUNCTIONS
 */
static void  ChatAddTimer (struct chatinfo *, char *, u_int, char *);
static void  ChatAddMatch (struct chatinfo *, char *, char *, char *);
static void  ChatCancel (struct chatinfo *c, char *set);
static int   ChatGoto (struct chatinfo *c, char *label);
static void  ChatCall (struct chatinfo *c, char *label);
static void  ChatLog (struct chatinfo *c, char *string);
static void  ChatPrint (struct chatinfo *c, char *string);
static void  ChatSend (struct chatinfo *c, char *string);
static void  ChatReturn (struct chatinfo *c, int seek);
static void  ChatRun (struct chatinfo *c);
static int   ChatCheck (struct chatinfo *c);
static void  ChatStop (struct chatinfo *c);
static void  ChatSuccess (struct chatinfo *c);
static void  ChatFailure (struct chatinfo *c);
static void  ChatIf (struct chatinfo *c, int ac, char *av[]);

static void  ChatDoCmd (struct chatinfo *c, int ac, char *av[]);
static void  ChatTimeout (void *arg);
static int   ChatGetCmd (char *token, int n_args);
static int   ChatVarExtract (const char *string, char *buf, int max, int strict);
static int   ChatMatchChar (char *target, int *howfar, char ch);
static int   ChatDecodeTime (struct chatinfo *c, char *string, u_int * secsp);
static char *ChatExpandString (struct chatinfo *c, const char *string);
static void  Escape   (char *line);
static char *ReadLine (FILE *fp);
static char *ReadFullLine (FILE *fp);
static void  FreeArgs (int ac, char **av);
static int   SeekToLabel (FILE *fp, char *label);
static int   ParseLine (char *line, char *av[], int max_args);

static struct chatvar *ChatVarGet (struct chatinfo *c, char *name);
static struct chatvar *ChatVarSet (struct chatinfo *c, char *name, char *value, int expand);
static struct chatvar *ChatVarFind (struct chatvar **head, char *name);

/*
 * ChatPresetVar()
 */
void ChatPresetVar (struct chatinfo *c, char *name, char *value)
{
  if (!ChatVarSet(c,name,value,1))
     cprintf ("invalid variable \"%s\"\r\n", name);
}

void ChatCallback (int stat)
{
  chat_okay = stat;
  chat_stop = 1;
}

int ChatMain (const char *script)
{
  struct chatinfo chat_info;

  cprintf ("running script `%s'\r\n", script);

  if (ChatStart(&chat_info,script,ChatCallback))
  {
    chat_stop = 0;
    chat_okay = 0;

    while (!chat_stop && !quit)
    {
      tcp_tick (&sock);
      if (ChatInput(&chat_info) < 0)
         break;
      delay (200);
      calltimeout();
    }
    return (chat_okay);
  }
  return (-1);
}

/*
 * Start executing chat script
 */
int ChatStart (struct chatinfo *c, const char *script, void (*result)(int))
{
  memset (c, 0, sizeof(*c));
  c->state = CHAT_IDLE;

  c->result = result;
  if (!c->result)
  {
    cprintf ("ChatStart() must have a callback\r\n");
    ChatFailure (c);
    return (0);
  }

  c->fp = fopen (script, "r");
  if (!c->fp)
  {
    cprintf ("can't open script file `%s'\r\n", script);
    ChatFailure (c);
    return (0);
  }

  ChatRun (c);

#if 1
  if (!ChatCheck(c))
     return (0);
#endif
  return (1);
}

/*
 *  Read TTY characters and match against script
 */
int ChatInput (struct chatinfo *c)
{
  struct chatmatch *match;
  char   label[CHAT_MAX_LABEL];
  int    read, ch = 0;

  /* Read one byte */
  read = GetCharTN (&ch);

  if (read < 0)       /* fatal error, or lines dropped */
     return (-1);

  if (read == 0)      /* no character within timeout */
     return (0);

  c->readBuf[c->readBufLen++] = ch;  /* add char to buffer */

  tel_dump ("ChatInput: `%s'\n", c->readBuf[c->readBufLen]);  // !!

  /* Try to match all match patterns
   */
  for (match = c->matches; match; match = match->next)
  {
    if (ChatMatchChar(match->pat, &match->n_matched, ch))
    {
      /* Save matched string in special variable
       */
      ChatVarSet (c, CHAT_VAR_MATCHED, match->pat, 0);

      cprintf ("matched set %s/%s/%s\r\n", match->set, match->pat, match->label);

      /* Remove set
       */
      sprintf (label, "%s", match->label);
      ChatCancel (c, match->set);

      /* Jump to target label
       */
      if (ChatGoto(c,label) >= 0)
         ChatRun (c);
      break;
    }
  }
  return (1);
}

/*
 * Handle one of our timers timing out
 */
static void ChatTimeout (void *arg)
{
  struct chattimer  *timer = (struct chattimer*) arg;
  struct chatinfo   *c     = timer->c;
  struct chattimer **tp;
  char   label[CHAT_MAX_LABEL];

  assert (c->state == CHAT_WAIT);

  /* Locate timer in list
   */
  for (tp = &c->timers; *tp != timer; tp = &(*tp)->next)
      ;

  assert (*tp);
  cprintf ("timer in set `%s' expired\r\n", timer->set);

  /* Cancel set
   */
  sprintf (label, "%s", timer->label);
  ChatCancel (c, timer->set);

  /* Jump to target label
   */
  if (ChatGoto(c,label) >= 0)
     ChatRun (c);
}

/*
 * Check a script for syntax errors. Must be called after ChatStart()
 */
static int ChatCheck (struct chatinfo *c)
{
  struct chatmatch *match;
  char   label[CHAT_MAX_LABEL];

  c->dry_run = 1;

  /* Loop through all match patterns
   */
  for (match = c->matches; match; match = match->next)
  {
    /* Save matched string in special variable
     */
    ChatVarSet (c, CHAT_VAR_MATCHED, match->pat, 0);

    cprintf ("testing set %s/%s/%s\r\n", match->set, match->pat, match->label);

    /* Remove set
     */
    sprintf (label, "%s", match->label);
    ChatCancel (c, match->set);

    /* Jump to target label
     */
    if (ChatGoto(c,label) >= 0)
       ChatRun (c);
  }
  c->dry_run = 0;
  return (1);
}

/*
 * Run chat script
 */
static void ChatRun (struct chatinfo *c)
{
  char *line;

  assert (c->state != CHAT_RUNNING);

  /* Cancel default set before running
   */
  ChatCancel (c, DEFAULT_SET);

  /* Execute commands while running
   */
  for (c->state = CHAT_RUNNING;
       c->state == CHAT_RUNNING && (line = ReadFullLine(c->fp)) != NULL; )
  {
    int   ac;
    char *av[CHAT_MAX_ARGS];

    /* Skip labels
     */
    if (!isspace(*line))
    {
      free (line);
      continue;
    }

    /* Parse out line
     */
    ac = ParseLine (line, av, CHAT_MAX_ARGS);
    free (line);

    /* Do command
     */
    ChatDoCmd (c, ac, av);
    FreeArgs (ac, av);
  }

  /* If EOF on file, assume failure
   */
  if (c->state == CHAT_RUNNING)
  {
    cprintf ("EOF while reading script\r\n");
    ChatFailure (c);
  }
  assert (c->state != CHAT_RUNNING);
}

/*
 * ChatDoCmd()
 */
static void ChatDoCmd (struct chatinfo *c, int ac, char *av[])
{
  u_int secs;
  int   k;

  /* Show command
   */
  if (dbug_mode || verbose >= 2)
  {
    char buf[200];

    for (buf[0] = k = 0; k < ac; k++)
    {
      int j;
      for (j = 0; av[k][j] && isgraph(av[k][j]); j++)
          ;
      sprintf (buf+strlen(buf), !*av[k] || av[k][j] ? " \"%s\"" : " %s",
               av[k]);
    }
    cprintf ("line %d:%s\r\n", lineNum, buf);
  }

  /* Execute command
   */
  if (ac == 0)
     return;

  switch (ChatGetCmd(av[0],ac))
  {
    case CMD_IF:
         ChatIf (c, ac, av);
         break;

    case CMD_SET:
         if (!ChatVarSet (c,av[1],av[2],1))
         {
           cprintf ("line %d: \" SET \": invalid variable `%s'\r\n",
                    lineNum, av[1]);
           ChatFailure (c);
         }
         break;

    case CMD_MATCH:
         switch (ac)
         {
           case 4:
                ChatAddMatch (c, av[1], av[2], av[3]);
                break;
           case 3:
                ChatAddMatch (c, DEFAULT_SET, av[1], av[2]);
                break;
           case 2:
                ChatAddMatch (c, DEFAULT_SET, av[1], DEFAULT_LABEL);
                break;
           default:
                assert (0);
         }
         break;

    case CMD_TIMER:
         switch (ac)
         {
           case 4:
                if (!ChatDecodeTime (c, av[2], &secs))
                   ChatFailure (c);
                else if (secs != 0)
                   ChatAddTimer (c, av[1], secs, av[3]);
                break;
           case 3:
                if (!ChatDecodeTime (c, av[1], &secs))
                   ChatFailure (c);
                else if (secs != 0)
                   ChatAddTimer (c, DEFAULT_SET, secs, av[2]);
                break;
           case 2:
                if (!ChatDecodeTime (c, av[1], &secs))
                   ChatFailure (c);
                else if (secs != 0)
                   ChatAddTimer (c, DEFAULT_SET, secs, DEFAULT_LABEL);
                break;
           default:
                assert (0);
         }
         break;

    case CMD_WAIT:
         if (ac == 2)
         {
           if (!ChatDecodeTime (c,av[1],&secs))
              ChatFailure (c);
           else if (secs > 0)
           {
             ChatAddTimer (c,DEFAULT_SET,secs,DEFAULT_LABEL);
             c->state = CHAT_WAIT;
           }
           else
             ChatCancel (c,DEFAULT_SET);  /* Simulate immediate expiration */
         }
         else
           c->state = CHAT_WAIT;
         break;

    case CMD_CANCEL:
         for (k = 1; k < ac; k++)
             ChatCancel (c, av[k]);
         break;

    case CMD_PRINT:
         ChatPrint (c, av[1]);
         break;

    case CMD_SEND:
         ChatSend (c, av[1]);
         break;

    case CMD_CALL:
         ChatCall (c, av[1]);
         break;

    case CMD_LOG:
         ChatLog (c, av[1]);
         break;

    case CMD_RETURN:
         ChatReturn (c, 1);
         break;

    case CMD_GOTO:
         ChatGoto (c, av[1]);
         break;

    case CMD_SUCCESS:
         ChatSuccess (c);
         break;

    case -1:
    case CMD_FAILURE:
         ChatFailure (c);
         break;

    default:
         assert (0);
  }
}

/*
 * ChatIf()
 */
static void ChatIf (struct chatinfo *c, int ac, char *av[])
{
  char *const arg1 = ChatExpandString (c, av[1]);
  char *const arg2 = ChatExpandString (c, av[3]);
  int   proceed    = 0;

  /* Check operator
   */
  if (!strcmp(av[2], "=="))
       proceed = !strcmp (arg1, arg2);
  else if (!strcmp (av[2],"!="))
       proceed = strcmp (arg1, arg2);
  else cprintf ("line %d: invalid operator `%s'\r\n", lineNum, av[2]);
  free (arg1);
  free (arg2);

  /* Do command
   */
  if (proceed)
     ChatDoCmd (c, ac-4, av+4);
}

/*
 * ChatSuccess()
 */
static void ChatSuccess (struct chatinfo *c)
{
  cprintf ("script succeeded\r\n");
  ChatStop (c);
  (*c->result) (TRUE);
}

/*
 * ChatFailure()
 */
static void ChatFailure (struct chatinfo *c)
{
  c->lastLog = NULL;
  cprintf ("script failed\r\n");
  ChatStop (c);
  (*c->result) (FALSE);
}

/*
 * ChatStop()
 */
static void ChatStop (struct chatinfo *c)
{
  struct chatvar *var, *next;

  /* Free temporary variables
   */
  for (var = c->temps; var; var = next)
  {
    next = var->next;
    free (var->name);
    free (var->value);
    free (var);
  }
  c->temps = NULL;
  if (c->fp)
  {
    fclose (c->fp);
    c->fp = NULL;
  }

  /* Cancel all sets
   */
  ChatCancel (c,CHAT_KEYWORD_ALL);

  /* Pop the stack
   */
  while (c->stack)
        ChatReturn (c, 0);

  /* Empty read buffer and last log message buffer
   */
  c->readBufLen = 0;
  if (c->lastLog)
  {
    free (c->lastLog);
    c->lastLog = NULL;
  }
  c->state = CHAT_IDLE;
}

/*
 * ChatAddMatch()
 */
static void ChatAddMatch (struct chatinfo *c, char *set, char *pat, char *label)
{
  struct chatmatch *match;

  /* Expand stuff
   */
  set   = ChatExpandString (c, set);
  pat   = ChatExpandString (c, pat);
  label = ChatExpandString (c, label);

  /* Create and add new match
   */
  match = calloc (sizeof(*match),1);
  sprintf (match->set, "%s", set);
  sprintf (match->pat, "%s", pat);
  sprintf (match->label, "%s", label);
  free (set);
  free (pat);
  free (label);

  match->n_matched = 0;
  match->next      = c->matches;
  c->matches = match;
}

/*
 * ChatAddTimer()
 */
static void ChatAddTimer (struct chatinfo *c, char *set, u_int secs, char *label)
{
  struct chattimer *timer;

  /* Expand stuff
   */
  set   = ChatExpandString (c, set);
  label = ChatExpandString (c, label);

  /* Add new timer
   */
  timer = calloc (sizeof(*timer),1);

  sprintf (timer->set, "%s", set);
  sprintf (timer->label, "%s", label);
  free (set);
  free (label);
  timer->c = c;

  timer->next = c->timers;
  c->timers = timer;

  timeout (ChatTimeout, (void*)timer, secs);
}

/*
 * ChatCancel()
 */
static void ChatCancel (struct chatinfo *c, char *set)
{
  int    all;
  char   setname[CHAT_MAX_NAME];
  struct chatmatch *match, **mp;
  struct chattimer *timer, **tp;

  /* Expand stuff
   */
  set = ChatExpandString (c, set);

  /* Save set name and check for special ALL keyword
   */
  sprintf (setname, "%s", set);
  free (set);
  all = (stricmp(setname,CHAT_KEYWORD_ALL) == 0);

  /* Nuke matches
   */
  for (mp = &c->matches; (match = *mp) != NULL; )
      if (all || !strcmp (match->set, setname))
      {
        *mp = match->next;
        free (match);
      }
      else
        mp = &match->next;

  /* Nuke timers
   */
  for (tp = &c->timers; (timer = *tp) != NULL; )
      if (all || !strcmp(timer->set, setname))
      {
        untimeout (ChatTimeout, (void*)timer);
        *tp = timer->next;
        free (timer);
      }
      else
        tp = &timer->next;
}

/*
 * ChatGoto()
 */
static int ChatGoto (struct chatinfo *c, char *label)
{
  int rtn;

  /* Expand label
   */
  label = ChatExpandString (c, label);

  /* Default label means "here"
   */
  if (!strcmp (label, DEFAULT_LABEL))
  {
    free (label);
    return (0);
  }

  /* Search script file
   */
  if ((rtn = SeekToLabel(c->fp, label)) < 0)
  {
    cprintf ("line %d: label `%s' not found\r\n", lineNum, label);
    ChatFailure (c);
  }
  free (label);
  return (rtn);
}

/*
 * ChatCall()
 */
static void ChatCall (struct chatinfo *c, char *label)
{
  struct chatframe *frame;

  /* Expand label
   */
  label = ChatExpandString (c, label);

  /* Adjust stack
   */
  frame = calloc (sizeof(*frame), 1);
  fgetpos (c->fp, &frame->posn);
  frame->line_num = lineNum;
  frame->up = c->stack;
  c->stack = frame;

  /* Find label
   */
  if (SeekToLabel (c->fp, label) < 0)
  {
    cprintf ("line %d: call label `%s' not found\r\n", lineNum, label);
    ChatFailure (c);
  }
  free (label);
}

/*
 * ChatReturn()
 */
static void ChatReturn (struct chatinfo *c, int seek)
{
  struct chatframe *frame;

  if (c->stack == NULL)
  {
    cprintf ("line %d: return without corresponding call\r\n", lineNum);
    ChatFailure (c);
    return;
  }

  /* Pop frame
   */
  frame = c->stack;
  if (seek)
  {
    fsetpos (c->fp, &frame->posn);
    lineNum = frame->line_num;
  }
  c->stack = frame->up;
  free (frame);
}

/*
 * ChatLog()
 */
static void ChatLog (struct chatinfo *c, char *string)
{
  char *exp_string = ChatExpandString (c, string);

  cprintf ("%s\r\n", exp_string);
  if (c->lastLog)
     free (c->lastLog);
  c->lastLog = exp_string;
}

/*
 * ChatPrint() - print a string to the console
 */
static void ChatPrint (struct chatinfo *c, char *string)
{
  char *exp_str = ChatExpandString (c, string);

  fputs (exp_str, stdout);
  free (exp_str);
}

/*
 * ChatSend() - send a string to the device
 */
static void ChatSend (struct chatinfo *c, char *string)
{
  char *exp_string = ChatExpandString (c, string);

  if (c->dry_run)
       cprintf ("ChatSend(%s)\r\n", exp_string);
  else PutStringTN (exp_string);
  free (exp_string);
}

/*
 * ChatVarSet()
 */
static struct chatvar *ChatVarSet (struct chatinfo *c, char *varname,
                                   char *value, int expand)
{
  struct chatvar *var, **head;
  char   *new, name[CHAT_MAX_VARNAME];

  /* Check & extract variable name
   */
  assert (varname && value);
  if (!*varname || ChatVarExtract(varname+1, name, sizeof(name), 1) < 0)
     return (NULL);

  head = isupper (*name) ? &c->globals : &c->temps;

  /* Create new value
   */
  if (expand)
       new = ChatExpandString (c, value);
  else new = strdup (value);

  /* Find variable if it exists, otherwise create and add to list
   */
  if ((var = ChatVarFind(head,name)) != NULL)
  {
    char *ovalue = var->value;  /* Replace value; free it after expanding */
    var->value = new;           /* (might be used in expansion)           */
    free (ovalue);
  }
  else
  {
    /* Create new struct and add to list
     */
    var = calloc (sizeof(*var),1);
    var->name  = strdup (name);
    var->value = new;
    var->next  = *head;
    *head = var;
  }
  return (var);
}

/*
 * ChatVarGet()
 */
static struct chatvar *ChatVarGet (struct chatinfo *c, char *varname)
{
  char name[CHAT_MAX_VARNAME];

  /* Check & extract variable name
   */
  if (!*varname || ChatVarExtract(varname+1, name, sizeof(name), 1) < 0)
     return (NULL);

  return ChatVarFind (isupper(*name) ? &c->globals : &c->temps, name);
}

/*
 * ChatVarFind()
 */
static struct chatvar *ChatVarFind (struct chatvar **head, char *name)
{
  struct chatvar *var, **vp;

  assert (head && name);
  for (vp = head; (var = *vp) != NULL && strcmp(var->name,name); vp = &var->next)
      ;
  if (var)
  {
    *vp = var->next;    /* caching */
    var->next = *head;
    *head = var;
  }
  return (var);
}

/*
 * ChatVarExtract()
 */
static int ChatVarExtract (const char *string, char *buf, int max, int strict)
{
  int k, len, paren;

  /* Get variable name. If surrounded by () return from environment
   */
  k = paren = (*string == '(');
  if (!isalpha(string[k]))
     return (-1);

  for (len = 0, k = paren;
       (isalnum(string[k]) || string[k] == '_') && len < max - 1;
       k++)
      buf[len++] = string[k];

  buf[len] = 0;
  if (len == 0 || (paren && string[k] != ')') || (strict && string[k+paren]))
     return (-1);
  return (len + paren*2);
}

/*
 * ChatGetCmd()
 */
static int ChatGetCmd (char *token, int n_args)
{
  unsigned k;

  for (k = 0; k < CHAT_NUM_COMMANDS; k++)
      if (!stricmp(commands[k].name, token))
      {
        if ((commands[k].min && n_args < commands[k].min) ||
            (commands[k].max && n_args > commands[k].max))
        {
          cprintf ("line %d: %s: bad argument count, "
                   "%d - %d arguments required\r\n",
                   lineNum, token, commands[k].min, commands[k].max);
          return (-1);
        }
        return (commands[k].id);
      }
  cprintf ("line %d: unknown command `%s'\r\n", lineNum, token);
  return (-1);
}

/*
 * ChatExpandString()
 *
 * Expand variables in string. Return result in Malloc'd buffer.
 */
static char * ChatExpandString (struct chatinfo *c, const char *string)
{
  struct chatvar *var;
  int    k, j, new_len, nlen, doit = 0;
  char  *new = NULL;
  char   nbuf[CHAT_MAX_VARNAME];

  /* Compute expanded length and then rescan and expand
   */
  new_len = 0;

rescan:
  for (j = k = 0; string[k]; k++)
    switch (string[k])
    {
      case CHAT_VAR_PREFIX:
	   *nbuf = string[k];
           nlen  = ChatVarExtract (string+k+1, nbuf+1, sizeof(nbuf)-1,0);
           if (nlen < 0)
              goto normal;
	   k += nlen;
           var = ChatVarGet (c,nbuf);
           if (var)
	   {
	     if (doit)
	     {
	       memcpy (new + j, var->value, strlen (var->value));
	       j += strlen (var->value);
	     }
	     else
	       new_len += strlen (var->value);
	   }
	   break;
      default:
      normal:
	   if (doit)
                new[j++] = string[k];
           else new_len++;
	   break;
    }

  /* Allocate and rescan
   */
  if (!doit)
  {
    new = calloc (new_len+1,1);
    doit = 1;
    goto rescan;
  }

  new[j] = 0;
  assert (j == new_len);
  return (new);
}

/*
 * If "howfar" is how many characters of the target we've already
 * matched, then update "howfar" given that "ch" is the next character.
 * Returns TRUE if target has become fully matched.
 *
 * Note: there are more efficient (ie linear time) ways to do this!
 */
static int ChatMatchChar (char *target, int *howfar, char ch)
{
  int rev;

  assert (target);

  for (rev = 0;
       rev <= *howfar && !(!strncmp(target,target+rev,*howfar-rev) &&
       ch == target[*howfar - rev]); rev++)
      ;
  *howfar -= rev - 1;
  return (!target[*howfar]);
}

/*
 * ChatDecodeTime()
 */
static int ChatDecodeTime (struct chatinfo *c, char *string, u_int * secsp)
{
  u_long secs;
  char  *secstr, *mark;

  secstr = ChatExpandString (c, string);
  secs   = strtoul (secstr, &mark, 0);
  free (secstr);
  if (mark == secstr)
  {
    cprintf ("line %d: illegal value `%s'\r\n", lineNum, string);
    return (0);
  }
  *secsp = (u_int) secs;
  return (1);
}

/*
 * Parse arguments, respecting double quotes and backslash escapes.
 * Returns number of arguments, at most "max_args". This destroys
 * the original line. The arguments returned are Malloc()'d strings
 * which must be freed by the caller using FreeArgs().
 */
static int ParseLine (char *line, char *av[], int max_args)
{
  int  ac;
  char *s, *arg;

  /* Get args one at a time
   */
  for (ac = 0; ac < max_args; ac++)
  {
    while (*line && isspace(*line))
       line++;

    if (*line == 0)
       break;

    /* Get normal or quoted arg
     */
    if (*line == '"')
    {
      /* Stop only upon matching quote or NUL
       */
      for (arg = ++line; *line; line++)
	if (*line == '"')
	{
	  *line++ = 0;
	  break;
	}
	else if (*line == '\\' && line[1] != 0)
	{
          strcpy (line, line + 1);
          Escape (line);
	}
      av[ac] = strdup (arg);
    }
    else
    {
      /* NUL terminate this argument at first white space
       */
      for (arg = line; *line && !isspace(*line); line++);
      if (*line)
          *line++ = 0;

      /* Convert characters
       */
      for (s = arg; *s; s++)
	if (*s == '\\')
	{
          strcpy (s, s + 1);
          Escape (s);
	}
      av[ac] = strdup (arg);
    }
  }
  return(ac);
}

/*
 * FreeArgs()
 */
static void FreeArgs (int ac, char **av)
{
  while (ac > 0)
    free (av[--ac]);
}

/*
 * Give a string, interpret the beginning characters as an escape
 * code and return with that code converted.
 */
static void Escape (char *line)
{
  int  x, k;
  char *s = line;

  switch (*line)
  {
    case 't':
         *s = '\t';
         return;
    case 'n':
         *s = '\n';
         return;
    case 'r':
         *s = '\r';
         return;
    case 's':
         *s =  ' ';
         return;
    case '"':
         *s =  '"';
         return;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
         for (s++, x = k = 0; k < 3 && *s >= '0' && *s <= '7'; s++)
             x = (x << 3) + (*s - '0');
         *--s = x;
         break;
    case 'x':
         for (s++, x = k = 0; k < 2 && isxdigit(*s); s++)
             x = (x << 4) + (isdigit(*s) ? (*s-'0') : (tolower(*s)-'a'+10));
         *--s = x;
         break;
    default:
         return;
  }
  strcpy (line, s);
}

/*
 * Find a label in file and position file pointer just after it
 */
static int SeekToLabel (FILE *fp, char *label)
{
  char *s, *line;

  /* Start at beginning
   */
  rewind (fp);
  lineNum = 0;

  /* Find label
   */
  while ((line = ReadFullLine(fp)) != NULL)
  {
    int	found;

    if (isspace(*line))
    {
      free (line);
      continue;
    }
    s = strtok (line, " \t\f:");
    found = (s && !strcmp(s,label));
    free (line);
    if (found)
       return (0);
  }
  return (-1);
}

/*
 * Read a full line, respecting backslash continuations.
 * Returns pointer to Malloc'd storage, which must be Freee'd
 */
static char *ReadFullLine (FILE *fp)
{
  int   len, continuation;
  char *real_line, *s;
  char  line[BIG_LINE_SIZE];

  for (*line = 0, continuation = 1; continuation; )
  {
    real_line = ReadLine (fp);
    if (!real_line)
    {
      if (*line)
         break;
      return (NULL);
    }

    for (s = real_line; *s; s++)  /* strip comments (#) */
        if (*s == '#')
        {
          *s = '\0';
          break;
        }

    /* Strip trailing white space, detect backslash
     */
    for (len = strlen(real_line);
         len > 0 && isspace(real_line[len - 1]);
         len--)
       real_line [len-1] = '\0';

    continuation = (*real_line && real_line[len-1] == '\\');
    if (continuation)
       real_line [len-1] = ' ';

    /* Append real line to what we've got so far
     */
    sprintf (line+strlen(line), real_line);
  }

  /* Report any overflow
   */
  if (strlen(line) >= sizeof(line) - 1)
     cprintf ("warning: line too long, truncated\r\n");

  return (strdup(line));
}

/*
 * Read a line, skipping blank lines & comments. A comment
 * is a line whose first non-white-space character is a hash.
 */
static char *ReadLine (FILE *fp)
{
  int    empty, ch;
  char  *s, *p;
  static char line[BIG_LINE_SIZE];

  /* Get first non-empty, non-commented line
   */
  for (empty = 1; empty; )
  {
    /* Read next line from file
     */
    if ((fgets(line, sizeof(line), fp)) == NULL)
       return (NULL);

    lineNum++;

    /* Truncate long lines
     */
    p = line + strlen(line) - 1;
    if (*p == '\n')
        *p = '\0';
    else
    {
      cprintf ("Warning: line too long, truncated\r\n");
      while ((ch = getc(fp)) != EOF && ch != '\n')
            ;
    }
    s = line + strspn (line, " \t");   /* Ignore comments */
    if (*s == '#')
        *s = '\0';

    /* Is this line empty?
     */
    for (empty = 1, s = line; *s; s++)
    {
      if (!isspace(*s))
      {
        empty = FALSE;
        break;
      }
    }
  }
  return (line);
}
