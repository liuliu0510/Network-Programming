The Server compiles to Server and the Client compiles to Client. 
Call server using:
./Server ipAddress port

Call client using:
./Client ipAddress port url

The url should be formatted such as www.amazon.com/ . The / is required. 

If a web page is retrieved without the last modified or expires header, than the page is assumed to be invalid and won't be used.

The Bonus credit for the Conditional Get was implemented. 