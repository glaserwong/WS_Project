#include <string>

class SecsParser
{
public:

	 SecsParser();
	 SecsParser( char* data, int data_len );

   void        SetSECS( char* data, int data_len );
   std::string GetXML();
   long        GetFunNum();
   long        GetSysByte();
   void        InitAttribute();

private:

	 std::string secs_xml;
	 long        funnum;
	 long        system_bytes;
};
