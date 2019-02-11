program Minimum_Size_Test;
{
 * test program for the Graph unit
}

uses
  Graph;

var
  grDriver, grMode: Integer;
begin
  grDriver := Detect;
  InitGraph(grDriver, grMode,'');
  CloseGraph
end.
