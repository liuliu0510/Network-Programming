#include "headers.h"

#define max 100

string to_string(int n)
{
	int m=n;
	int i=0,j=0;
	char s[max];
	char ss[max];
	while(m>0)
	{
		s[i++]= m%10 + '0';
		m/=10;
	}
	s[i]='\0';

	i=i-1;
	while(i>=0)
	{
		ss[j++]=s[i--];
	}
	ss[j]='\0';

	return ss;
}


class Date
{
private:
	string Day_Week ;
	int Year;
	int Month;
	int Day;
	int Hour;
	int Minute;
	int Second;
	
public:
	Date()
        :Day_Week(""), Year(-1), Month(-1), Day(-1), Hour(-1), Minute(-1), Second(-1){}
 
    Date(int year, int month, int day, int hour, int minute, int second)
		:Year (year), Month(month), Day (day), Hour(hour), Minute(minute), Second(second) {}

	//determines if a given date is valid or not
	bool Valid()
	{
		return Year  != -1;
	}

	//constructor given a time_t
	Date(time_t time)
	{
		tm timeTm = *(gmtime(&time));
		Day_Week  = DayOfTheWeek(timeTm.tm_wday);
		Day  = timeTm.tm_mday;
		Month = timeTm.tm_mon;
		Year  = timeTm.tm_year + 1900;
		Hour = timeTm.tm_hour;
		Minute = timeTm.tm_min;
		Second = timeTm.tm_sec;
	}

	//returning a date in the needed html format
	string HtmlFormat()
	{
		string result = "";
		
		result += Day_Week ;
		result += ", ";
		if (Day  < 10)
			result += "0";
		result += to_string(Day);
		result += " ";
		result += Get_Month_Str(Month);
		result += " ";
		result += to_string(Year);
		result += " ";
		if (Hour < 10)
			result += "0";
		result += to_string(Hour);
		result += ":";
		if (Minute < 10)
			result += "0";
		result += to_string(Minute);
		result += ":";
		if (Second < 10)
			result += "0";
		result += to_string(Second);
		result += " GMT";

		return result;
	}

	//given a number, return the corresponding name 
	string DayOfTheWeek(int num)
	{ 
	string str;
	switch(num)
	   {
		case 0:
			str = "Sun";
			break;
		case 1:
		   	str = "Mon";		
			break;
		case 2:
			str = "Tue";		
			break;
		case 3:
			str = "Wed";		
			break;
		case 4:
			str = "Thur";	
			break;
		case 5:
			str = "Fri";	
			break;
		case 6:
			str = "Sat";	
			break;
     	}	
	 return str;
	}

	//given a string in html format, extract the date info
	Date(string msg)
	{
		if (msg== "")
		{
			Day_Week  = "";
			Day  = -1;
			Month = -1;
			Year  = -1;
			Hour = -1;
			Minute = -1;
			Second = -1;
		}
		else
		{
			Find_Colon(msg);

			Day_Week  = Get_next_Str(msg);
			Day_Week.erase(Day_Week .size() - 1, Day_Week .size());
			Day  = Get_next_Int(msg);
			string month =Get_next_Str(msg);
			Month = Get_Month_Num(month);
			Year  = Get_next_Int(msg);
			Hour = Get_next_Int(msg);
			Minute = Get_next_Int(msg);
			Second = Get_next_Int(msg);
		}
	}

	//add one to a given date
	Date Add_one_Day(Date date)
	{
		Date result = Date(
			date.Year ,
			date.Month,
			date.Day  + 1,
			date.Hour,
			date.Minute,
			date.Second);
		
		//result.Print(); 
		return result;
	}

	//subtract a month from the given date
	Date Subtract_one_Month(Date date)
	{
		int temp_month = date.Month;
		int temp_year = date.Year ;
		temp_month--;
		if (temp_month == 0)
		{
			temp_month = 12;
			temp_year--;
		}

		Date result = Date(
			temp_year,
			temp_month,
			date.Day ,
			date.Hour,
			date.Minute,
			date.Second);
		return result;
	}

	//determine if this date is earlier than the given date
	bool EarlierThan(Date date)
	{
		if (this->Year  < date.Year )
			return true;
		if (this->Year  > date.Year )
			return false;

		if (this->Month < date.Month)
			return true;
		if (this->Month > date.Month)
			return false;

		if (this->Day  < date.Day )
			return true;
		if (this->Day  > date.Day )
			return false;

		if (this->Hour < date.Hour)
			return true;
		if (this->Hour > date.Hour)
			return false;

		if (this->Minute < date.Minute)
			return true;
		if (this->Minute > date.Minute)
			return false;

		if (this->Second < date.Second)
			return true;
		if (this->Second > date.Second)
			return false;

		return false;
	}

	//finds the next integer in a string
	int Get_next_Int(string& msg)
	{

		bool cont = true;
		int num = 0;

		while (cont)
			if (isdigit(msg[0]))
				cont = false;
			else
				msg.erase(0, 1);

		cont = true;
		while (cont)
		{
			if (isdigit(msg[0]))
				num = num * 10 + msg[0] - '0';
			else
				cont = false;

			msg.erase(0, 1);
		}

		return num;
	}


	//retrieves the next string in a given string
	string Get_next_Str(string& msg)
	{
		bool cont = true;
		string result = "";

		while (cont)
			if (msg[0] != ' ' & !isdigit(msg[0]))
				cont = false;
			else
				msg.erase(0, 1);

		cont = true;
		while (cont)
		{
			if (msg[0] != ' ')
				result += msg[0];
			else
				cont = false;

			msg.erase(0, 1);
		}

		return result;
	}

	//gos to the next colon
	void Find_Colon(string& msg)
	{
		while (msg[0] != ':')
		{
			msg.erase(0, 1);
		}
		msg.erase(0, 1);
	}

	//given a number, return the string
	string Get_Month_Str(int num)
	{
			string str = "";
	switch(num)
	   {
		case 1:
			str = "Jan"; break;
		case 2:
		   	str = "Feb"; break;
		case 3:
			str = "Mar"; break;		
		case 4:
			str = "Apr"; break;		
		case 5:
			str = "May"; break;		
		case 6:
			str = "Jun"; break;		
		case 7:
			str = "Jul"; break;	
		case 8:
			str = "Aug"; break;	
		case 9:
			str = "Sep"; break;	
		case 10:
			str = "Oct"; break;	
		case 11:
			str = "Nov"; break;	
		case 12:
			str = "Dec"; break;	
     	}	
	 return str;
    }

	//given a Month string, returns the corresponding number
	int Get_Month_Num(string msg)
	{
		if (msg == "Jan")
			return 1;
		if (msg == "Feb")
			return 2;
		if (msg == "Mar")
			return 3;
		if (msg == "Apr")
			return 4;
		if (msg == "May")
			return 5;
		if (msg == "Jun")
			return 6;
		if (msg == "Jul")
			return 7;
		if (msg == "Aug")
			return 8;
		if (msg == "Sep")
			return 9;
		if (msg == "Oct")
			return 10;
		if (msg == "Nov")
			return 11;
		if (msg == "Dec")
			return 12;

		return -1;
	}



	void Print()
	{
		cout << "Year: " << Year  << endl;
		cout << "Month: " << Month << endl;
		cout << "Day: " << Day  << endl;
		cout << "Day of the Week: " << Day_Week  << endl;
		cout << "Hour: " << Hour << endl;
		cout << "Minute: " << Minute << endl;
		cout << "Second: " << Second << endl;
	}

};
