RULE(ALPHANUM,
     ONE(RANGE('a', 'z') || RANGE('A', 'Z') || RANGE('0', '9')))

RULE(DOMAINLABEL_MINUS,	/* alphanum *( alphanum | "-" ) alphanum */
     ONE(CALL(ALPHANUM))
     MANY(CALL(ALPHANUM) || STRING("-")))

RULE(TOPLABEL_MINUS,	/* alpha *( alphanum | "-" ) alphanum */
     ONE(RANGE('a', 'z') || RANGE('A', 'Z'))
     MANY(CALL(ALPHANUM) || STRING("-")))

RULE(DOMAINLABEL,	/* alphanum | alphanum *( alphanum | "-" ) alphanum */
     ONE(CALL(DOMAINLABEL_MINUS) || CALL(ALPHANUM)))

RULE(TOPLABEL,		/* alpha | alpha *( alphanum | "-" ) alphanum */
     ONE(CALL(TOPLABEL_MINUS) || RANGE('a', 'z') || RANGE('A', 'Z')))

RULE(HOSTNAME,	/* *( domainlabel "." ) toplabel [ "." ] */
     MANY(CALL(DOMAINLABEL) && STRING("."))
     ONE(CALL(TOPLABEL))
     OPTIONAL(STRING(".")))

RULE(RESERVED,		/* [;/?:@&=+$,] */
     ONE(LIST(";/?:@&=+$,")))

RULE(UNRESERVED,	/* alphanum | [-_.!~*'()] */
     ONE(RANGE('a', 'z') || RANGE('A', 'Z') ||
	 RANGE('0', '9') || LIST("-_.!~*'()")))

RULE(ESCAPED,		/* "%" hex hex */
     ONE(STRING("%"))
     ONE(CALL(HEX))
     ONE(CALL(HEX)))

RULE(URIC,		/* reserved | unreserved | escaped */
     ONE(CALL(RESERVED) || CALL(UNRESERVED) || CALL(ESCAPED)))

RULE(QUERY,		/* *uric */
     MANY(CALL(URIC)))

RULE(REG_NAME,	/* 1*( unreserved | escaped | [$,;:@&=+]) */
     AT_LEAST_ONE(CALL(UNRESERVED) || CALL(ESCAPED) || LIST("$,;:@&=+")))

RULE(REL_SEGMENT,	/* 1*( unreserved | escaped | [;@&=+$,] */
     AT_LEAST_ONE(CALL(UNRESERVED) || CALL(ESCAPED) || LIST(";@&=+$,")))

RULE(USERINFO,	/* *( unreserved | escaped | [;:&=+$,] ) */
     MANY(CALL(UNRESERVED) || CALL(ESCAPED) || LIST(";:&=+$,")))

RULE(O_USERINFO_AT,	/* [ userinfo "@" ] */
     OPTIONAL(CALL(USERINFO) && STRING("@")))

RULE(IPV4ADDRESS,	/* 1*digit "." 1*digit "." 1*digit "." 1*digit */
     AT_LEAST_ONE(RANGE('0', '9'))
     ONE(STRING("."))
     AT_LEAST_ONE(RANGE('0', '9'))
     ONE(STRING("."))
     AT_LEAST_ONE(RANGE('0', '9'))
     ONE(STRING("."))
     AT_LEAST_ONE(RANGE('0', '9')))

RULE(HOST,	/* hostname | IPv4address */
     ONE(CALL(HOSTNAME) || CALL(IPV4ADDRESS)))

RULE(PORT,		/* *digit */
     MANY(RANGE('0', '9')))

RULE(HOSTPORT,	/* host [ ":" port ] */
     ONE(CALL(HOST))
     OPTIONAL(STRING(":") && CALL(PORT)))

RULE(SERVER,	/* [ [ userinfo "@" ] hostport ] */
     OPTIONAL(CALL(O_USERINFO_AT) && CALL(HOSTPORT)))

RULE(AUTHORITY,	/* server | reg_name */
     ONE(CALL(SERVER) || CALL(REG_NAME)))

RULE(PCHAR,		/* unreserved | escaped | [:@&=+$,] */
     ONE(CALL(UNRESERVED) || CALL(ESCAPED) || LIST(":@&=+$,")))

RULE(PARAM,		/* *pchar */
     MANY(CALL(PCHAR)))

RULE(SEGMENT,		/* *pchar *( ";" param ) */
     MANY(CALL(PCHAR))
     MANY(STRING(";") && CALL(PARAM)))

RULE(PATH_SEGMENTS,	/* segment *( "/" segment ) */
     ONE(CALL(SEGMENT))
     MANY(STRING("/") && CALL(SEGMENT)))

RULE(ABS_PATH,	/* "/"  path_segments */ /* Rajoute par mandark : ?query*/
     ONE(STRING("/"))
     ONE(CALL(PATH_SEGMENTS))
     OPTIONAL(CALL(QUERY)))

RULE(NET_PATH,	/* "//" authority [ abs_path ] */
     ONE(STRING("//"))
     ONE(CALL(AUTHORITY))
     OPTIONAL(CALL(ABS_PATH)))

RULE(REL_PATH,	/* rel_segment [ abs_path ] */
     ONE(CALL(REL_SEGMENT))
     OPTIONAL(CALL(ABS_PATH)))

RULE(RELATIVE_URI,	/* ( net_path | abs_path | rel_path ) [ "?" query ] */
     ONE(CALL(NET_PATH) || CALL(ABS_PATH) || CALL(REL_PATH))
     OPTIONAL(STRING("?") && CALL(QUERY)))

RULE(FRAGMENT,		/* *uric */
     MANY(CALL(URIC)))

RULE(HIER_PART,	/* ( net_path | abs_path ) [ "?" query ] */
     ONE(CALL(NET_PATH) || CALL(ABS_PATH))
     OPTIONAL(STRING("?") && CALL(QUERY)))

RULE(URIC_NO_SLASH, /* unreserved | escaped | [;?:@&=+$,] */
     ONE(CALL(UNRESERVED) || CALL(ESCAPED) || LIST(";?:@&=+$,")))

RULE(OPAQUE_PART, /* uric_no_slash *uric */
     ONE(CALL(URIC_NO_SLASH))
     MANY(CALL(URIC)))

RULE(SCHEME,	/* alpha *( alpha | digit | "+" | "-" | "." ) */
     ONE(RANGE('a', 'z')
         || RANGE('A', 'Z'))
     MANY(RANGE('a', 'z')
          || RANGE('A', 'Z')
          || RANGE('0', '9')
          || LIST("+-.")))

RULE(ABSOLUTE_URI,	/* scheme ":" ( hier_part | opaque_part )	*/
     ONE(CALL(SCHEME))
     ONE(STRING(":"))
     ONE(CALL(HIER_PART) || CALL(OPAQUE_PART)))

RULE(PATH,		/* [ abs_path | opaque_part ] */
     OPTIONAL(CALL(ABS_PATH) || CALL(OPAQUE_PART)))

RULE(URI_REFERENCE,	/* [ absoluteURI | relativeURI ] [ "#" fragment ] */
     OPTIONAL(CALL(ABSOLUTE_URI) || CALL(RELATIVE_URI))
     OPTIONAL(STRING("#") && CALL(FRAGMENT)))

