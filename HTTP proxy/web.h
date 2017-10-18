#include "headers.h"

//stores a URL given from the client
class URL
{
private:
	string Domain; //domain of the URL from the client
	string Page;

public:
	URL(){
		Domain = "";
		Page = "";
	}

	URL(string msg)
	{
		int pos = 0;
		Domain = "";
		Page = "";

        //constructs the domain
		while (msg[pos] != '/') // slash that separates the domain and page
		{
			Domain += msg[pos];
			pos++;
		}
		//constructs the page
		while (pos < msg.size())
		{
			Page += msg[pos];
			pos++;
		}
	}
	string Get_Domain() { return Domain; }
	string Get_Page() { return Page; }
};


//class for the retrieved webpage
class WebPage
{
private:
	URL Url;
	string Body;
	bool Empty;
	bool has_ExpireDate;
	bool has_LastModified;
	Date Accesstime;
	Date ExpiresDate;
	Date LastModifiedDate;
	string Header;

public:
    WebPage(){
    	Url = URL();
    	Body = "";
    	Empty = true;
    	Accesstime = time(0);
    }
    WebPage(string url){
    	Url = URL(url);
    	Body = "";
    	Empty = true;
    	Accesstime = time(0);
    }

    URL GetUrl() { return Url; }

    //get the expires message in the header
	string Get_Expires_Msg(string msg)
	{
		string cur_msg = "";
		int pos = 0;
		bool value = true;

		//look for the string "Expires"
		while (value)
		{
			cur_msg+= msg[pos];

			//once the end of the line is found, analyze the line
			if (msg[pos] == '\n')
			{
				if (cur_msg[0] == 'E')
					if (cur_msg[1] == 'x')
						if (cur_msg[2] == 'p')
							if (cur_msg[3] == 'i')
								if (cur_msg[4] == 'r')
									if (cur_msg[5] == 'e')
										if (cur_msg[6] == 's')
											return cur_msg;
				cur_msg = "";
			}
			//once you get to the '<' you have reached the body
			if (msg[pos] == '<')
				value = false;
			pos++;
		}
		//if it was not found, just returned a blank string
		return "";
	}

    	//looks for the last modified message
	string Get_LastModified_Msg(string msg)
	{
		string cur_msg = "";
		int pos = 0;
		bool value = true;

		//look for the string "Last-Modified"
		while (value)
		{
			cur_msg += msg[pos];

			//once the end of the line is found, analyze the line
			if (msg[pos] == '\n')
			{
				if (cur_msg[0] == 'L')
					if (cur_msg[1] == 'a')
						if (cur_msg[2] == 's')
							if (cur_msg[3] == 't')
								if (cur_msg[4] == '-')
									if (cur_msg[5] == 'M')
										if (cur_msg[6] == 'o')
											if (cur_msg[7] == 'd')
												if (cur_msg[8] == 'i')
													if (cur_msg[9] == 'f')
														if (cur_msg[10] == 'i')
															if (cur_msg[11] == 'e')
																if (cur_msg[12] == 'd')
																	return cur_msg;
				cur_msg = "";
			}
			//once you get to the '<' you have reached the body
			if (msg[pos] == '<')
				value = false;
			pos++;
		}
		//if it was not found, just returned a blank string
		return "";
	}


	//function that actually does the page retrieval
	string Get_Page()
	{
		//if the needed page is still valid, then just return it
		if (IsValid(true))
		{
			//record the current time it is accessed
			time_t access = time(0);
			Accesstime = Date(access);
			cout << "Page returned from cache:" << endl;
			return Body;
		}

		cout << "Page must access server for the data" << endl;

		string received_msg = ""; //the string that the incoming page gets stored in

		int sockfd1; //socket  connect to the domain
		struct addrinfo hints, *servInfo, *p; //information to loop through the values returned
		int a;

	    memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
		hints.ai_socktype = SOCK_STREAM;
		if ((a = getaddrinfo(Url.Get_Domain().c_str(), "http", &hints, &servInfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(a));
			exit(1);
		}

		// loop through all the results and connect to the first we can
		for (p = servInfo; p != NULL; p = p->ai_next) {
			if ((sockfd1 = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			{
				perror("socket");
				continue;
			}

			if (connect(sockfd1, p->ai_addr, p->ai_addrlen) == -1)
			{
				close(sockfd1);
				perror("connect");
				continue;
			}

			break; // if we get here, we must have connected successfully
		}

		if (p == NULL) {
			// looped off the end of the list with no connection
			fprintf(stderr, "failed to connect\n");
			exit(2);
		}

		char buf[buffer_size];
		memset(&buf, 0, sizeof(buf)); //clear out the buffer
		string msg = "";

		if (Empty)
		{
			//if the page has not yet been retrieved
			msg = "GET " + Url.Get_Page() + " HTTP/1.0\r\n"
				"Host: " + Url.Get_Domain() + "\r\n"
				"User-Agent: HTMLGET 1.0\r\n"
				"\r\n";
		}
		else
		{
			//if the page has already been retrieved, and was invalid
			//it will do a conditional get
			string htmlFormat = Accesstime.HtmlFormat();
			msg = "GET " + Url.Get_Page() + " HTTP/1.0\r\n"
				"Host: " + Url.Get_Domain() + "\r\n"
				"User-Agent: HTMLGET 1.0\r\n"
				"If-Modified-Since: " + htmlFormat + "\r\n"
				"\r\n";
		}

		//record the current time it will be accessed
		time_t access = time(0);
		Accesstime = Date(access);

		//write message to the socket
		for (int i = 0; i < msg.size(); i++)
			buf[i] = msg[i];
		int bytes_sent = send(sockfd1, buf, msg.size(),0);

		//read the message returned from the socket
		int bytes_recv = 1;
		while (bytes_recv > 0)
		{
			memset(&buf, 0, sizeof(buf));
		    bytes_recv = recv(sockfd1, buf, sizeof(buf),0);

			for (int i = 0; i < bytes_recv; i++)
				received_msg += buf[i];
		}
		received_msg += EOF; //adding the EOF character, will be detected by the client

		//determine if the message was a 304 that has not been updated
		if (Contains304(received_msg))
		{
			cout << "Page was invalid, however the Conditional Get returned that it had not been updated." << endl;
			return Body;
		}

		//split the received message into the header and body
		Split(received_msg);

		//determine if there was an expiration date, and set it
		string expiration = Get_Expires_Msg(Header);
		ExpiresDate = Date(expiration);
		has_ExpireDate = ExpiresDate.Valid();

		//determine if there was a last modified date, and set it
		string lastmodified = Get_LastModified_Msg(Header);
		LastModifiedDate = Date(lastmodified);
		has_LastModified = LastModifiedDate.Valid();

		//free resources
		freeaddrinfo(servInfo); // all done with this structure
		close(sockfd1);

		Empty = false; //mark that the page has been accessed


		//return the html document body
		return Body;
	}

	//determines if the header has the 304 that it has not been updated
	bool Contains304(string msg)
	{
		string firstline = ""; // makes the first line of the header
		int pos = 0;

		//loops through to create the first line of the header
		while (msg[pos] != '\n')
		{
			firstline += msg[pos];
			pos++;
		}

		//goes through the first line looking for the 304
		int num = 0;
		for (int i = 0; i < firstline.size(); i++)
		{
			if (isdigit(firstline[i]))
				num  = 10 * num  + firstline[i] - '0';
			else
				num = 0;

			if (num  == 304)
				return true;
		}

		return false;
	}

	//determine if the page was valid
	bool IsValid(bool print)
	{
		//if the page has not been gotten, return false
		if (Empty)
			return false;
		Date cur_time = Date(time(0)); //the current time
		//if there is an expired header, see if it is earlier than the current time
		if (has_ExpireDate)
		{
			if (cur_time.EarlierThan(ExpiresDate))
			{
				if (print)
				{
					cout << "Page is valid. Access is still before the expiration date" << endl;
				}
				return true;
			}
		}
		//if the last modified date was
		if (has_LastModified)
		{
			if (LastModifiedDate.Subtract_one_Month(cur_time).EarlierThan(cur_time))
			{
				if (cur_time.EarlierThan(Accesstime.Add_one_Day(Accesstime)))
				{
					if (print)
					{
						cout << "Page is valid. Access is less than a day since last access, and "
							<< "last modified date is more than a month ago" << endl;

					}
					return true;
				}
			}
		}

		if (print)
		{
			cout << "Page is no longer valid" << endl;
		}
		return false;
	}

	Date GetAccessTime() { return Accesstime; }

	//split the the message received into the header and body
	void Split(string msg)
	{
		int pos = 0;
		Header = "";
		Body = "";

		//adds to the header until the '<' is found
		while (msg[pos] != '<')
		{
			Header += msg[pos];
			pos++;
		}

		//adds to the body the rest of the string
		while (pos < msg.size())
		{
			Body+= msg[pos];
			pos++;
		}
	}


};


//The actual cache, implements the replacement policy
class Cache
{
private:
	vector<WebPage> Pages; //the vector holding all pages in the cache

public:
	//initializes the whole cache with a blank page
	Cache(int cachesize)
	{
		for (int i = 0; i < cachesize; i++)
		{
			Pages.push_back(WebPage("www.amazon.com/"));
			Pages.push_back(WebPage("www.yahoo.com/"));
			Pages.push_back(WebPage("www.tamu.edu/"));
			Pages.push_back(WebPage("www.ebay.com/"));
			Pages.push_back(WebPage("www.baidu.com/"));
		/*	Pages.push_back(WebPage("www.amazon.co.jp/"));
		    Pages.push_back(WebPage("www.live.com"));
			Pages.push_back(WebPage("www.sina.com.cn/"));
			Pages.push_back(WebPage("www.qq.com/"));
			Pages.push_back(WebPage("www.taobao.com/")); */ 
	     //Pages.push_back(WebPage());
		}
	}

	//gets a page out of the cache
	string Get_Page(string urlstr)
	{
		cout << endl << endl << "Retreiving - " << urlstr << endl;
		URL url(urlstr); //get the url from the given string

		//look to see if the page is already in the cache
		for (int i = 0; i < Pages.size(); i++)
			if (Pages[i].GetUrl().Get_Domain() == url.Get_Domain()
				&Pages[i].GetUrl().Get_Page() == url.Get_Page())
			{
				cout << "Page was found in the cache" << endl;
				return Pages[i].Get_Page();
			}

		//if not in the cache, see if there is a page that is no longer valid in the cache
		for (int i = 0; i <Pages.size(); i++)
			if (!Pages[i].IsValid(false))
			{
				cout << "Page was not found in the cache, but there was an empty or invalid block found" << endl;
				Pages[i] = WebPage(urlstr);
				return Pages[i].Get_Page();
			}

		//if they are all filled, replace the least recently used
		Date mintime = Date(time(0));
		int pos = -1;
		for (int i = 0; i <Pages.size(); i++)
			if (Pages[i].GetAccessTime().EarlierThan(mintime))
			{
				mintime =Pages[i].GetAccessTime();
				pos = i;
			}

		cout << "Page was not found in cache, and the least recently blocked was replaced" << endl;
		Pages[pos] = WebPage(urlstr);
		return Pages[pos].Get_Page();
	}

};
