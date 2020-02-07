------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--                     Terminal_Interface.Curses.Terminfo                   --
--                                                                          --
--                                 S P E C                                  --
--                                                                          --
------------------------------------------------------------------------------
-- Copyright (c) 2000,2003 Free Software Foundation, Inc.                   --
--                                                                          --
-- Permission is hereby granted, free of charge, to any person obtaining a  --
-- copy of this software and associated documentation files (the            --
-- "Software"), to deal in the Software without restriction, including      --
-- without limitation the rights to use, copy, modify, merge, publish,      --
-- distribute, distribute with modifications, sublicense, and/or sell       --
-- copies of the Software, and to permit persons to whom the Software is    --
-- furnished to do so, subject to the following conditions:                 --
--                                                                          --
-- The above copyright notice and this permission notice shall be included  --
-- in all copies or substantial portions of the Software.                   --
--                                                                          --
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  --
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               --
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   --
-- IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   --
-- DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    --
-- OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    --
-- THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               --
--                                                                          --
-- Except as contained in this notice, the name(s) of the above copyright   --
-- holders shall not be used in advertising or otherwise to promote the     --
-- sale, use or other dealings in this Software without prior written       --
-- authorization.                                                           --
------------------------------------------------------------------------------
--  Author:  Juergen Pfeifer, 1996
--  Version Control:
--  $Revision: 1.4 $
--  Binding Version 01.00
------------------------------------------------------------------------------

with Interfaces.C;

package Terminal_Interface.Curses.Terminfo is
   pragma Preelaborate (Terminal_Interface.Curses.Terminfo);

   --  |=====================================================================
   --  | Man page curs_terminfo.3x
   --  |=====================================================================
   --  Not implemented:  setupterm, setterm, set_curterm, del_curterm,
   --                    restartterm, tparm, putp, vidputs,  vidattr,
   --                    mvcur

   type Terminfo_String is new String;

   --  |
   procedure Get_String (Name   : String;
                         Value  : out Terminfo_String;
                         Result : out Boolean);
   function Has_String (Name : String) return Boolean;
   --  AKA: tigetstr()

   --  |
   function Get_Flag (Name : String) return Boolean;
   --  AKA: tigetflag()

   --  |
   function Get_Number (Name : String) return Integer;
   --  AKA: tigetnum()

   type putctype is access function (c : Interfaces.C.int)
                                    return Interfaces.C.int;
   pragma Convention (C, putctype);

   --  |
   procedure Put_String (Str    : Terminfo_String;
                         affcnt : Natural := 1;
                         putc   : putctype := null);
   --  AKA: tputs()

end Terminal_Interface.Curses.Terminfo;
