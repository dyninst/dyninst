
/* a struct with one int */
$remote typedef struct { 
	String hostname;
	int family;
	int port;
	int type;
} xdr_socketStruct;

/*
 *
 */
$remote xdr_link {
	$base 2000;
	$version 1;
	void connected(String msg);
};
