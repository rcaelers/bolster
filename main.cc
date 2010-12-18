// main.cc --- OAuth test app
//
// Copyright (C) 2010 Rob Caelers <robc@krandor.org>
// All rights reserved.

#include "OAuth.hh"

int main(int argc, char **argv)
{
  OAuth oauth("Hello", "World", "http://api.krandor.org/callback", "HMAC-SHA1");
  oauth.request_temporary_credentials("GET", "http://api.krandor.org/login");
  
  oauth.request("GET", "http://api.krandor.org:80/login?user=robc&x=&email=robc%40krandor.org", "Token", "Secret");
  
}
