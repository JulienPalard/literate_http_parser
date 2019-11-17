#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <uthash.h>
#include <string.h>
#include "parser.h"


/*  RFC 2616 */
/*  ======== */

/* 2 Notational Conventions and Generic Grammar */

/* 2.1 Augmented BNF */

/*    All of the mechanisms specified in this document are described in */
/*    both prose and an augmented Backus-Naur Form (BNF) similar to that */
/*    used by RFC 822 [9]. Implementors will need to be familiar with the */
/*    notation in order to understand this specification. The augmented BNF */
/*    includes the following constructs: */

/*    name = definition */
/*       The name of a rule is simply the name itself (without any */
/*       enclosing "<" and ">") and is separated from its definition by the */
/*       equal "=" character. White space is only significant in that */
/*       indentation of continuation lines is used to indicate a rule */
/*       definition that spans more than one line. Certain basic rules are */
/*       in uppercase, such as SP, LWS, HT, CRLF, DIGIT, ALPHA, etc. Angle */
/*       brackets are used within definitions whenever their presence will */
/*       facilitate discerning the use of rule names. */

/* // Implemented by RULE(name, definition) */

/*    "literal" */
/*       Quotation marks surround literal text. Unless stated otherwise, */
/*       the text is case-insensitive. */

/* // Like a literal in C but be carefull that it's case-insensitive */

/*    rule1 | rule2 */
/*       Elements separated by a bar ("|") are alternatives, e.g., "yes | */
/*       no" will accept yes or no. */

/* // Implemented by rule1 || rule2 in C */

/*    (rule1 rule2) */
/*       Elements enclosed in parentheses are treated as a single element. */
/*       Thus, "(elem (foo | bar) elem)" allows the token sequences "elem */
/*       foo elem" and "elem bar elem". */

/* // Implemented by (rule1 && rule2) in C */

/*    *rule */
/*       The character "*" preceding an element indicates repetition. The */
/*       full form is "<n>*<m>element" indicating at least <n> and at most */
/*       <m> occurrences of element. Default values are 0 and infinity so */
/*       that "*(element)" allows any number, including zero; "1*element" */
/*       requires at least one; and "1*2element" allows one or two. */

/* // *element is implemented by MANY(element) */
/* // 1*element is implemented by AT_LEAST_ONE(element) */

/*       [rule] */
/*          Square brackets enclose optional elements; "[foo bar]" is */
/*          equivalent to "*1(foo bar)". */

/* // Implemented by OPTIONAL(rule) */

/*       N rule */
/*          Specific repetition: "<n>(element)" is equivalent to */
/*          "<n>*<n>(element)"; that is, exactly <n> occurrences of (element). */
/*          Thus 2DIGIT is a 2-digit number, and 3ALPHA is a string of three */
/*          alphabetic characters. */

/*       #rule */
/*          A construct "#" is defined, similar to "*", for defining lists of */
/*          elements. The full form is "<n>#<m>element" indicating at least */
/*          <n> and at most <m> elements, each separated by one or more commas */
/*          (",") and OPTIONAL linear white space (LWS). This makes the usual */
/*          form of lists very easy; a rule such as */
/*             ( *LWS element *( *LWS "," *LWS element )) */
/*          can be shown as */
/*             1#element */
/*          Wherever this construct is used, null elements are allowed, but do */
/*          not contribute to the count of elements present. That is, */
/*          "(element), , (element) " is permitted, but counts as only two */
/*          elements. Therefore, where at least one element is required, at */
/*          least one non-null element MUST be present. Default values are 0 */
/*          and infinity so that "#element" allows any number, including zero; */
/*          "1#element" requires at least one; and "1#2element" allows one or */
/*          two. */

/*       ; comment */
/*          A semi-colon, set off some distance to the right of rule text, */
/*          starts a comment that continues to the end of line. This is a */
/*          simple way of including useful notes in parallel with the */
/*          specifications. */

/*       implied *LWS */
/*          The grammar described by this specification is word-based. Except */
/*          where noted otherwise, linear white space (LWS) can be included */
/*          between any two adjacent words (token or quoted-string), and */
/*          between adjacent words and separators, without changing the */
/*          interpretation of a field. At least one delimiter (LWS and/or */

/*          separators) MUST exist between any two tokens (for the definition */
/*          of "token" below), since they would otherwise be interpreted as a */
/*          single token. */

/* 2.2 Basic Rules */

/*    The following rules are used throughout this specification to */
/*    describe basic parsing constructs. The US-ASCII coded character set */
/*    is defined by ANSI X3.4-1986 [21]. */

/*           OCTET          = <any 8-bit sequence of data> */
RULE(OCTET,
     ONE(RANGE(0, (char)255)))

/*        CHAR           = <any US-ASCII character (octets 0 - 127)> */
RULE(CHAR,
     ONE(RANGE(0, 127)))

/*        UPALPHA        = <any US-ASCII uppercase letter "A".."Z"> */
RULE(UPALPHA,
     ONE(RANGE('A', 'Z')))

/*        LOALPHA        = <any US-ASCII lowercase letter "a".."z"> */
RULE(LOALPHA,
     ONE(RANGE('a', 'z')))

/*        ALPHA          = UPALPHA | LOALPHA */
RULE(ALPHA,
     ONE(RANGE('a', 'z') || RANGE('A', 'Z')))

/*        DIGIT          = <any US-ASCII digit "0".."9"> */
RULE(DIGIT,
     ONE(RANGE('0', '9')))
/*        CTL            = <any US-ASCII control character
                        (octets 0 - 31) and DEL (127)> */
RULE(CTL,
     ONE(RANGE(0, 31)))

/*        CR             = <US-ASCII CR, carriage return (13)> */
RULE(CR,
     ONE(CHAR(13)))

/*        LF             = <US-ASCII LF, linefeed (10)> */
RULE(LF,
     ONE(CHAR(10)))

/*        SP             = <US-ASCII SP, space (32)> */
RULE(SP,
     ONE(CHAR(32)))

/*        HT             = <US-ASCII HT, horizontal-tab (9)> */
RULE(HT,
     ONE(CHAR(9)))

/*        <">            = <US-ASCII double-quote mark (34)> */
RULE(DBL_QUOTE,
     ONE(CHAR(34)))

/*    HTTP/1.1 defines the sequence CR LF as the end-of-line marker for all */
/*    protocol elements except the entity-body (see appendix 19.3 for */
/*    tolerant applications). The end-of-line marker within an entity-body */
/*    is defined by its associated media type, as described in section 3.7. */

/*        CRLF           = CR LF */
RULE(CRLF,
     ONE(STRING("\r\n")))

/*    HTTP/1.1 header field values can be folded onto multiple lines if the */
/*    continuation line begins with a space or horizontal tab. All linear */
/*    white space, including folding, has the same semantics as SP. A */
/*    recipient MAY replace any linear white space with a single SP before */
/*    interpreting the field value or forwarding the message downstream. */

/*        LWS            = [CRLF] 1*( SP | HT ) */
RULE(LWS,
     OPTIONAL(STRING("\r\n"))
     AT_LEAST_ONE(STRING(" ") || STRING("\t")))

/*    The TEXT rule is only used for descriptive field contents and values */
/*    that are not intended to be interpreted by the message parser. Words */
/*    of *TEXT MAY contain characters from character sets other than ISO- */
/*    8859-1 [22] only when encoded according to the rules of RFC 2047 */
/*    [14]. */

/*        TEXT           = <any OCTET except CTLs, */
/*                         but including LWS> */

RULE(TEXT, MANY(RANGE(32, (char)255)
                || CALL(LWS)))

/*    A CRLF is allowed in the definition of TEXT only as part of a header */
/*    field continuation. It is expected that the folding LWS will be */
/*    replaced with a single SP before interpretation of the TEXT value. */

/*    Hexadecimal numeric characters are used in several protocol elements. */

/*        HEX            = "A" | "B" | "C" | "D" | "E" | "F" */
/*                       | "a" | "b" | "c" | "d" | "e" | "f" | DIGIT */

RULE(HEX, ONE(RANGE('0', '9')
              || RANGE('A','F')
              || RANGE('a', 'f')))


/*    Many HTTP/1.1 header field values consist of words separated by LWS */
/*    or special characters. These special characters MUST be in a quoted */
/*    string to be used within a parameter value (as defined in section */
/*    3.6). */

/*        token          = 1*<any CHAR except CTLs or separators> */

RULE(TOKEN,
     AT_LEAST_ONE(RANGE('0', '9')
                  || RANGE('A', 'Z')
                  || RANGE('a', 'z')
                  || LIST("*!+^|#-_$.‘~%&’")))

/*        separators     = "(" | ")" | "<" | ">" | "@" */
/*                       | "," | ";" | ":" | "\" | <"> */
/*                       | "/" | "[" | "]" | "?" | "=" */
/*                       | "{" | "}" | SP | HT */

/*    Comments can be included in some HTTP header fields by surrounding */
/*    the comment text with parentheses. Comments are only allowed in */
/*    fields containing "comment" as part of their field value definition. */
/*    In all other fields, parentheses are considered part of the field */
/*    value. */

/*        comment        = "(" *( ctext | quoted-pair | comment ) ")" */
DECLARE_RULE(CTEXT)
DECLARE_RULE(QUOTED_PAIR)
RULE(COMMENT,
     ONE(CHAR('('))
     MANY(CALL(CTEXT)
          || CALL(QUOTED_PAIR)
          || CALL(COMMENT))
     ONE(CHAR('(')))

/*        ctext          = <any TEXT excluding "(" and ")"> */
RULE(CTEXT,
     MANY(RANGE(32, 39)
          || RANGE(42, (char)255)
          || CALL(LWS)))

/*    A string of text is parsed as a single word if it is quoted using */
/*    double-quote marks. */

/*        quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> ) */
DECLARE_RULE(QDTEXT)
RULE(QUOTED_STRING,
     ONE(CHAR('"'))
     MANY(CALL(QDTEXT) || CALL(QUOTED_PAIR))
     ONE(CHAR('"')))

/*        qdtext         = <any TEXT except <">> */
RULE(QDTEXT, MANY(RANGE(32, 33)
                  || RANGE(35, (char)255)
                  || CALL(LWS)))

/*    The backslash character ("\") MAY be used as a single-character */
/*    quoting mechanism only within quoted-string and comment constructs. */

/*     quoted-pair    = "\" CHAR */
RULE(QUOTED_PAIR, ONE(CHAR('\\')) CALL(CHAR))


/* 3 Protocol Parameters */

/* 3.1 HTTP Version */

/*    HTTP uses a "<major>.<minor>" numbering scheme to indicate versions */
/*    of the protocol. The protocol versioning policy is intended to allow */
/*    the sender to indicate the format of a message and its capacity for */
/*    understanding further HTTP communication, rather than the features */
/*    obtained via that communication. No change is made to the version */
/*    number for the addition of message components which do not affect */
/*    communication behavior or which only add to extensible field values. */
/*    The <minor> number is incremented when the changes made to the */
/*    protocol add features which do not change the general message parsing */
/*    algorithm, but which may add to the message semantics and imply */
/*    additional capabilities of the sender. The <major> number is */
/*    incremented when the format of a message within the protocol is */
/*    changed. See RFC 2145 [36] for a fuller explanation. */

/*    The version of an HTTP message is indicated by an HTTP-Version field */
/*    in the first line of the message. */

/*        HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT */

RULE(HTTP_VERSION,
     ONE(STRING("HTTP/"))
     AT_LEAST_ONE(RANGE('0', '9'))
     ONE(STRING("."))
     AT_LEAST_ONE(RANGE('0', '9')))

/*    Note that the major and minor numbers MUST be treated as separate */
/*    integers and that each MAY be incremented higher than a single digit. */
/*    Thus, HTTP/2.4 is a lower version than HTTP/2.13, which in turn is */
/*    lower than HTTP/12.3. Leading zeros MUST be ignored by recipients and */
/*    MUST NOT be sent. */

/*    An application that sends a request or response message that includes */
/*    HTTP-Version of "HTTP/1.1" MUST be at least conditionally compliant */
/*    with this specification. Applications that are at least conditionally */
/*    compliant with this specification SHOULD use an HTTP-Version of */
/*    "HTTP/1.1" in their messages, and MUST do so for any message that is */
/*    not compatible with HTTP/1.0. For more details on when to send */
/*    specific HTTP-Version values, see RFC 2145 [36]. */

/*    The HTTP version of an application is the highest HTTP version for */
/*    which the application is at least conditionally compliant. */

/*    Proxy and gateway applications need to be careful when forwarding */
/*    messages in protocol versions different from that of the application. */
/*    Since the protocol version indicates the protocol capability of the */
/*    sender, a proxy/gateway MUST NOT send a message with a version */
/*    indicator which is greater than its actual version. If a higher */
/*    version request is received, the proxy/gateway MUST either downgrade */
/*    the request version, or respond with an error, or switch to tunnel */
/*    behavior. */

/*    Due to interoperability problems with HTTP/1.0 proxies discovered */
/*    since the publication of RFC 2068[33], caching proxies MUST, gateways */
/*    MAY, and tunnels MUST NOT upgrade the request to the highest version */
/*    they support. The proxy/gateway's response to that request MUST be in */
/*    the same major version as the request. */

/*       Note: Converting between versions of HTTP may involve modification */
/*       of header fields required or forbidden by the versions involved. */

/* 3.2 Uniform Resource Identifiers */

/*    URIs have been known by many names: WWW addresses, Universal Document */
/*    Identifiers, Universal Resource Identifiers [3], and finally the */
/*    combination of Uniform Resource Locators (URL) [4] and Names (URN) */
/*    [20]. As far as HTTP is concerned, Uniform Resource Identifiers are */
/*    simply formatted strings which identify--via name, location, or any */
/*    other characteristic--a resource. */

/* 3.2.1 General Syntax */

/*    URIs in HTTP can be represented in absolute form or relative to some */
/*    known base URI [11], depending upon the context of their use. The two */
/*    forms are differentiated by the fact that absolute URIs always begin */
/*    with a scheme name followed by a colon. For definitive information on */
/*    URL syntax and semantics, see "Uniform Resource Identifiers (URI): */
/*    Generic Syntax and Semantics," RFC 2396 [42] (which replaces RFCs */
/*    1738 [4] and RFC 1808 [11]). This specification adopts the */
/*    definitions of "URI-reference", "absoluteURI", "relativeURI", "port", */
/*    "host","abs_path", "rel_path", and "authority" from that */
/*    specification. */

#include "uri_parser.c"

/*    The HTTP protocol does not place any a priori limit on the length of */
/*    a URI. Servers MUST be able to handle the URI of any resource they */
/*    serve, and SHOULD be able to handle URIs of unbounded length if they */
/*    provide GET-based forms that could generate such URIs. A server */
/*    SHOULD return 414 (Request-URI Too Long) status if a URI is longer */
/*    than the server can handle (see section 10.4.15). */

/*       Note: Servers ought to be cautious about depending on URI lengths */
/*       above 255 bytes, because some older client or proxy */
/*       implementations might not properly support these lengths. */

/* 3.2.2 http URL */

/*    The "http" scheme is used to locate network resources via the HTTP */
/*    protocol. This section defines the scheme-specific syntax and */
/*    semantics for http URLs. */

/*    http_URL = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]] */

/*    If the port is empty or not given, port 80 is assumed. The semantics */
/*    are that the identified resource is located at the server listening */
/*    for TCP connections on that port of that host, and the Request-URI */
/*    for the resource is abs_path (section 5.1.2). The use of IP addresses */
/*    in URLs SHOULD be avoided whenever possible (see RFC 1900 [24]). If */
/*    the abs_path is not present in the URL, it MUST be given as "/" when */
/*    used as a Request-URI for a resource (section 5.1.2). If a proxy */
/*    receives a host name which is not a fully qualified domain name, it */
/*    MAY add its domain to the host name it received. If a proxy receives */
/*    a fully qualified domain name, the proxy MUST NOT change the host */
/*    name. */

/* 3.2.3 URI Comparison */

/*    When comparing two URIs to decide if they match or not, a client */
/*    SHOULD use a case-sensitive octet-by-octet comparison of the entire */
/*    URIs, with these exceptions: */

/*       - A port that is empty or not given is equivalent to the default */
/*         port for that URI-reference; */

/*         - Comparisons of host names MUST be case-insensitive; */

/*         - Comparisons of scheme names MUST be case-insensitive; */

/*         - An empty abs_path is equivalent to an abs_path of "/". */

/*    Characters other than those in the "reserved" and "unsafe" sets (see */
/*    RFC 2396 [42]) are equivalent to their ""%" HEX HEX" encoding. */

/*    For example, the following three URIs are equivalent: */

/*       http://abc.com:80/~smith/home.html */
/*       http://ABC.com/%7Esmith/home.html */
/*       http://ABC.com:/%7esmith/home.html */

/* 3.3 Date/Time Formats */

/* 3.3.1 Full Date */

/*    HTTP applications have historically allowed three different formats */
/*    for the representation of date/time stamps: */

/*       Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123 */
/*       Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036 */
/*       Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format */

/*    The first format is preferred as an Internet standard and represents */
/*    a fixed-length subset of that defined by RFC 1123 [8] (an update to */
/*    RFC 822 [9]). The second format is in common use, but is based on the */
/*    obsolete RFC 850 [12] date format and lacks a four-digit year. */
/*    HTTP/1.1 clients and servers that parse the date value MUST accept */
/*    all three formats (for compatibility with HTTP/1.0), though they MUST */
/*    only generate the RFC 1123 format for representing HTTP-date values */
/*    in header fields. See section 19.3 for further information. */

/*       Note: Recipients of date values are encouraged to be robust in */
/*       accepting date values that may have been sent by non-HTTP */
/*       applications, as is sometimes the case when retrieving or posting */
/*       messages via proxies/gateways to SMTP or NNTP. */

/*    All HTTP date/time stamps MUST be represented in Greenwich Mean Time */
/*    (GMT), without exception. For the purposes of HTTP, GMT is exactly */
/*    equal to UTC (Coordinated Universal Time). This is indicated in the */
/*    first two formats by the inclusion of "GMT" as the three-letter */
/*    abbreviation for time zone, and MUST be assumed when reading the */
/*    asctime format. HTTP-date is case sensitive and MUST NOT include */
/*    additional LWS beyond that specifically included as SP in the */
/*    grammar. */

/*        HTTP-date    = rfc1123-date | rfc850-date | asctime-date */
/*        rfc1123-date = wkday "," SP date1 SP time SP "GMT" */
/*        rfc850-date  = weekday "," SP date2 SP time SP "GMT" */
/*        asctime-date = wkday SP date3 SP time SP 4DIGIT */
/*        date1        = 2DIGIT SP month SP 4DIGIT */
/*                       ; day month year (e.g., 02 Jun 1982) */
/*        date2        = 2DIGIT "-" month "-" 2DIGIT */
/*                       ; day-month-year (e.g., 02-Jun-82) */
/*        date3        = month SP ( 2DIGIT | ( SP 1DIGIT )) */
/*                       ; month day (e.g., Jun  2) */
/*        time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT */
/*                       ; 00:00:00 - 23:59:59 */
/*        wkday        = "Mon" | "Tue" | "Wed" */
/*                     | "Thu" | "Fri" | "Sat" | "Sun" */
/*        weekday      = "Monday" | "Tuesday" | "Wednesday" */
/*                     | "Thursday" | "Friday" | "Saturday" | "Sunday" */
/*        month        = "Jan" | "Feb" | "Mar" | "Apr" */
/*                     | "May" | "Jun" | "Jul" | "Aug" */
/*                     | "Sep" | "Oct" | "Nov" | "Dec" */

/*       Note: HTTP requirements for the date/time stamp format apply only */
/*       to their usage within the protocol stream. Clients and servers are */
/*       not required to use these formats for user presentation, request */
/*       logging, etc. */

/* 3.3.2 Delta Seconds */

/*    Some HTTP header fields allow a time value to be specified as an */
/*    integer number of seconds, represented in decimal, after the time */
/*    that the message was received. */

/*        delta-seconds  = 1*DIGIT */

/* 3.4 Character Sets */

/*    HTTP uses the same definition of the term "character set" as that */
/*    described for MIME: */

/*    The term "character set" is used in this document to refer to a */
/*    method used with one or more tables to convert a sequence of octets */
/*    into a sequence of characters. Note that unconditional conversion in */
/*    the other direction is not required, in that not all characters may */
/*    be available in a given character set and a character set may provide */
/*    more than one sequence of octets to represent a particular character. */
/*    This definition is intended to allow various kinds of character */
/*    encoding, from simple single-table mappings such as US-ASCII to */
/*    complex table switching methods such as those that use ISO-2022's */
/*    techniques. However, the definition associated with a MIME character */
/*    set name MUST fully specify the mapping to be performed from octets */
/*    to characters. In particular, use of external profiling information */
/*    to determine the exact mapping is not permitted. */

/*       Note: This use of the term "character set" is more commonly */
/*       referred to as a "character encoding." However, since HTTP and */
/*       MIME share the same registry, it is important that the terminology */
/*       also be shared. */

/*    HTTP character sets are identified by case-insensitive tokens. The */
/*    complete set of tokens is defined by the IANA Character Set registry */
/*    [19]. */

/*        charset = token */

/*    Although HTTP allows an arbitrary token to be used as a charset */
/*    value, any token that has a predefined value within the IANA */
/*    Character Set registry [19] MUST represent the character set defined */
/*    by that registry. Applications SHOULD limit their use of character */
/*    sets to those defined by the IANA registry. */

/*    Implementors should be aware of IETF character set requirements [38] */
/*    [41]. */

/* 3.4.1 Missing Charset */

/*    Some HTTP/1.0 software has interpreted a Content-Type header without */
/*    charset parameter incorrectly to mean "recipient should guess." */
/*    Senders wishing to defeat this behavior MAY include a charset */
/*    parameter even when the charset is ISO-8859-1 and SHOULD do so when */
/*    it is known that it will not confuse the recipient. */

/*    Unfortunately, some older HTTP/1.0 clients did not deal properly with */
/*    an explicit charset parameter. HTTP/1.1 recipients MUST respect the */
/*    charset label provided by the sender; and those user agents that have */
/*    a provision to "guess" a charset MUST use the charset from the */

/*    content-type field if they support that charset, rather than the */
/*    recipient's preference, when initially displaying a document. See */
/*    section 3.7.1. */

/* 3.5 Content Codings */

/*    Content coding values indicate an encoding transformation that has */
/*    been or can be applied to an entity. Content codings are primarily */
/*    used to allow a document to be compressed or otherwise usefully */
/*    transformed without losing the identity of its underlying media type */
/*    and without loss of information. Frequently, the entity is stored in */
/*    coded form, transmitted directly, and only decoded by the recipient. */

/*        content-coding   = token */

/*    All content-coding values are case-insensitive. HTTP/1.1 uses */
/*    content-coding values in the Accept-Encoding (section 14.3) and */
/*    Content-Encoding (section 14.11) header fields. Although the value */
/*    describes the content-coding, what is more important is that it */
/*    indicates what decoding mechanism will be required to remove the */
/*    encoding. */

/*    The Internet Assigned Numbers Authority (IANA) acts as a registry for */
/*    content-coding value tokens. Initially, the registry contains the */
/*    following tokens: */

/*    gzip An encoding format produced by the file compression program */
/*         "gzip" (GNU zip) as described in RFC 1952 [25]. This format is a */
/*         Lempel-Ziv coding (LZ77) with a 32 bit CRC. */

/*    compress */
/*         The encoding format produced by the common UNIX file compression */
/*         program "compress". This format is an adaptive Lempel-Ziv-Welch */
/*         coding (LZW). */

/*         Use of program names for the identification of encoding formats */
/*         is not desirable and is discouraged for future encodings. Their */
/*         use here is representative of historical practice, not good */
/*         design. For compatibility with previous implementations of HTTP, */
/*         applications SHOULD consider "x-gzip" and "x-compress" to be */
/*         equivalent to "gzip" and "compress" respectively. */

/*    deflate */
/*         The "zlib" format defined in RFC 1950 [31] in combination with */
/*         the "deflate" compression mechanism described in RFC 1951 [29]. */

/*    identity */
/*         The default (identity) encoding; the use of no transformation */
/*         whatsoever. This content-coding is used only in the Accept- */
/*         Encoding header, and SHOULD NOT be used in the Content-Encoding */
/*         header. */

/*    New content-coding value tokens SHOULD be registered; to allow */
/*    interoperability between clients and servers, specifications of the */
/*    content coding algorithms needed to implement a new value SHOULD be */
/*    publicly available and adequate for independent implementation, and */
/*    conform to the purpose of content coding defined in this section. */

/* 3.6 Transfer Codings */

/*    Transfer-coding values are used to indicate an encoding */
/*    transformation that has been, can be, or may need to be applied to an */
/*    entity-body in order to ensure "safe transport" through the network. */
/*    This differs from a content coding in that the transfer-coding is a */
/*    property of the message, not of the original entity. */

/*        transfer-coding         = "chunked" | transfer-extension */
/*        transfer-extension      = token *( ";" parameter ) */

/*    Parameters are in  the form of attribute/value pairs. */

/*        parameter               = attribute "=" value */
/*        attribute               = token */
/*        value                   = token | quoted-string */

/*    All transfer-coding values are case-insensitive. HTTP/1.1 uses */
/*    transfer-coding values in the TE header field (section 14.39) and in */
/*    the Transfer-Encoding header field (section 14.41). */

/*    Whenever a transfer-coding is applied to a message-body, the set of */
/*    transfer-codings MUST include "chunked", unless the message is */
/*    terminated by closing the connection. When the "chunked" transfer- */
/*    coding is used, it MUST be the last transfer-coding applied to the */
/*    message-body. The "chunked" transfer-coding MUST NOT be applied more */
/*    than once to a message-body. These rules allow the recipient to */
/*    determine the transfer-length of the message (section 4.4). */

/*    Transfer-codings are analogous to the Content-Transfer-Encoding */
/*    values of MIME [7], which were designed to enable safe transport of */
/*    binary data over a 7-bit transport service. However, safe transport */
/*    has a different focus for an 8bit-clean transfer protocol. In HTTP, */
/*    the only unsafe characteristic of message-bodies is the difficulty in */
/*    determining the exact body length (section 7.2.2), or the desire to */
/*    encrypt data over a shared transport. */

/*    The Internet Assigned Numbers Authority (IANA) acts as a registry for */
/*    transfer-coding value tokens. Initially, the registry contains the */
/*    following tokens: "chunked" (section 3.6.1), "identity" (section */
/*    3.6.2), "gzip" (section 3.5), "compress" (section 3.5), and "deflate" */
/*    (section 3.5). */

/*    New transfer-coding value tokens SHOULD be registered in the same way */
/*    as new content-coding value tokens (section 3.5). */

/*    A server which receives an entity-body with a transfer-coding it does */
/*    not understand SHOULD return 501 (Unimplemented), and close the */
/*    connection. A server MUST NOT send transfer-codings to an HTTP/1.0 */
/*    client. */

/* 3.6.1 Chunked Transfer Coding */

/*    The chunked encoding modifies the body of a message in order to */
/*    transfer it as a series of chunks, each with its own size indicator, */
/*    followed by an OPTIONAL trailer containing entity-header fields. This */
/*    allows dynamically produced content to be transferred along with the */
/*    information necessary for the recipient to verify that it has */
/*    received the full message. */

/*        Chunked-Body   = *chunk */
/*                         last-chunk */
/*                         trailer */
/*                         CRLF */

/*        chunk          = chunk-size [ chunk-extension ] CRLF */
/*                         chunk-data CRLF */
/*        chunk-size     = 1*HEX */
/*        last-chunk     = 1*("0") [ chunk-extension ] CRLF */

/*        chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] ) */
/*        chunk-ext-name = token */
/*        chunk-ext-val  = token | quoted-string */
/*        chunk-data     = chunk-size(OCTET) */
/*        trailer        = *(entity-header CRLF) */

/*    The chunk-size field is a string of hex digits indicating the size of */
/*    the chunk. The chunked encoding is ended by any chunk whose size is */
/*    zero, followed by the trailer, which is terminated by an empty line. */

/*    The trailer allows the sender to include additional HTTP header */
/*    fields at the end of the message. The Trailer header field can be */
/*    used to indicate which header fields are included in a trailer (see */
/*    section 14.40). */

/*    A server using chunked transfer-coding in a response MUST NOT use the */
/*    trailer for any header fields unless at least one of the following is */
/*    true: */

/*    a)the request included a TE header field that indicates "trailers" is */
/*      acceptable in the transfer-coding of the  response, as described in */
/*      section 14.39; or, */

/*    b)the server is the origin server for the response, the trailer */
/*      fields consist entirely of optional metadata, and the recipient */
/*      could use the message (in a manner acceptable to the origin server) */
/*      without receiving this metadata.  In other words, the origin server */
/*      is willing to accept the possibility that the trailer fields might */
/*      be silently discarded along the path to the client. */

/*    This requirement prevents an interoperability failure when the */
/*    message is being received by an HTTP/1.1 (or later) proxy and */
/*    forwarded to an HTTP/1.0 recipient. It avoids a situation where */
/*    compliance with the protocol would have necessitated a possibly */
/*    infinite buffer on the proxy. */

/*    An example process for decoding a Chunked-Body is presented in */
/*    appendix 19.4.6. */

/*    All HTTP/1.1 applications MUST be able to receive and decode the */
/*    "chunked" transfer-coding, and MUST ignore chunk-extension extensions */
/*    they do not understand. */

/* 3.7 Media Types */

/*    HTTP uses Internet Media Types [17] in the Content-Type (section */
/*    14.17) and Accept (section 14.1) header fields in order to provide */
/*    open and extensible data typing and type negotiation. */

/*        media-type     = type "/" subtype *( ";" parameter ) */
/*        type           = token */
/*        subtype        = token */

/*    Parameters MAY follow the type/subtype in the form of attribute/value */
/*    pairs (as defined in section 3.6). */

/*    The type, subtype, and parameter attribute names are case- */
/*    insensitive. Parameter values might or might not be case-sensitive, */
/*    depending on the semantics of the parameter name. Linear white space */
/*    (LWS) MUST NOT be used between the type and subtype, nor between an */
/*    attribute and its value. The presence or absence of a parameter might */
/*    be significant to the processing of a media-type, depending on its */
/*    definition within the media type registry. */

/*    Note that some older HTTP applications do not recognize media type */
/*    parameters. When sending data to older HTTP applications, */
/*    implementations SHOULD only use media type parameters when they are */
/*    required by that type/subtype definition. */

/*    Media-type values are registered with the Internet Assigned Number */
/*    Authority (IANA [19]). The media type registration process is */
/*    outlined in RFC 1590 [17]. Use of non-registered media types is */
/*    discouraged. */

/* 3.7.1 Canonicalization and Text Defaults */

/*    Internet media types are registered with a canonical form. An */
/*    entity-body transferred via HTTP messages MUST be represented in the */
/*    appropriate canonical form prior to its transmission except for */
/*    "text" types, as defined in the next paragraph. */

/*    When in canonical form, media subtypes of the "text" type use CRLF as */
/*    the text line break. HTTP relaxes this requirement and allows the */
/*    transport of text media with plain CR or LF alone representing a line */
/*    break when it is done consistently for an entire entity-body. HTTP */
/*    applications MUST accept CRLF, bare CR, and bare LF as being */
/*    representative of a line break in text media received via HTTP. In */
/*    addition, if the text is represented in a character set that does not */
/*    use octets 13 and 10 for CR and LF respectively, as is the case for */
/*    some multi-byte character sets, HTTP allows the use of whatever octet */
/*    sequences are defined by that character set to represent the */
/*    equivalent of CR and LF for line breaks. This flexibility regarding */
/*    line breaks applies only to text media in the entity-body; a bare CR */
/*    or LF MUST NOT be substituted for CRLF within any of the HTTP control */
/*    structures (such as header fields and multipart boundaries). */

/*    If an entity-body is encoded with a content-coding, the underlying */
/*    data MUST be in a form defined above prior to being encoded. */

/*    The "charset" parameter is used with some media types to define the */
/*    character set (section 3.4) of the data. When no explicit charset */
/*    parameter is provided by the sender, media subtypes of the "text" */
/*    type are defined to have a default charset value of "ISO-8859-1" when */
/*    received via HTTP. Data in character sets other than "ISO-8859-1" or */
/*    its subsets MUST be labeled with an appropriate charset value. See */
/*    section 3.4.1 for compatibility problems. */

/* 3.7.2 Multipart Types */

/*    MIME provides for a number of "multipart" types -- encapsulations of */
/*    one or more entities within a single message-body. All multipart */
/*    types share a common syntax, as defined in section 5.1.1 of RFC 2046 */

/*    [40], and MUST include a boundary parameter as part of the media type */
/*    value. The message body is itself a protocol element and MUST */
/*    therefore use only CRLF to represent line breaks between body-parts. */
/*    Unlike in RFC 2046, the epilogue of any multipart message MUST be */
/*    empty; HTTP applications MUST NOT transmit the epilogue (even if the */
/*    original multipart contains an epilogue). These restrictions exist in */
/*    order to preserve the self-delimiting nature of a multipart message- */
/*    body, wherein the "end" of the message-body is indicated by the */
/*    ending multipart boundary. */

/*    In general, HTTP treats a multipart message-body no differently than */
/*    any other media type: strictly as payload. The one exception is the */
/*    "multipart/byteranges" type (appendix 19.2) when it appears in a 206 */
/*    (Partial Content) response, which will be interpreted by some HTTP */
/*    caching mechanisms as described in sections 13.5.4 and 14.16. In all */
/*    other cases, an HTTP user agent SHOULD follow the same or similar */
/*    behavior as a MIME user agent would upon receipt of a multipart type. */
/*    The MIME header fields within each body-part of a multipart message- */
/*    body do not have any significance to HTTP beyond that defined by */
/*    their MIME semantics. */

/*    In general, an HTTP user agent SHOULD follow the same or similar */
/*    behavior as a MIME user agent would upon receipt of a multipart type. */
/*    If an application receives an unrecognized multipart subtype, the */
/*    application MUST treat it as being equivalent to "multipart/mixed". */

/*       Note: The "multipart/form-data" type has been specifically defined */
/*       for carrying form data suitable for processing via the POST */
/*       request method, as described in RFC 1867 [15]. */

/* 3.8 Product Tokens */

/*    Product tokens are used to allow communicating applications to */
/*    identify themselves by software name and version. Most fields using */
/*    product tokens also allow sub-products which form a significant part */
/*    of the application to be listed, separated by white space. By */
/*    convention, the products are listed in order of their significance */
/*    for identifying the application. */

/*        product         = token ["/" product-version] */
/*        product-version = token */

/*    Examples: */

/*        User-Agent: CERN-LineMode/2.15 libwww/2.17b3 */
/*        Server: Apache/0.8.4 */

/*    Product tokens SHOULD be short and to the point. They MUST NOT be */
/*    used for advertising or other non-essential information. Although any */
/*    token character MAY appear in a product-version, this token SHOULD */
/*    only be used for a version identifier (i.e., successive versions of */
/*    the same product SHOULD only differ in the product-version portion of */
/*    the product value). */

/* 3.9 Quality Values */

/*    HTTP content negotiation (section 12) uses short "floating point" */
/*    numbers to indicate the relative importance ("weight") of various */
/*    negotiable parameters.  A weight is normalized to a real number in */
/*    the range 0 through 1, where 0 is the minimum and 1 the maximum */
/*    value. If a parameter has a quality value of 0, then content with */
/*    this parameter is `not acceptable' for the client. HTTP/1.1 */
/*    applications MUST NOT generate more than three digits after the */
/*    decimal point. User configuration of these values SHOULD also be */
/*    limited in this fashion. */

/*        qvalue         = ( "0" [ "." 0*3DIGIT ] ) */
/*                       | ( "1" [ "." 0*3("0") ] ) */

/*    "Quality values" is a misnomer, since these values merely represent */
/*    relative degradation in desired quality. */

/* 3.10 Language Tags */

/*    A language tag identifies a natural language spoken, written, or */
/*    otherwise conveyed by human beings for communication of information */
/*    to other human beings. Computer languages are explicitly excluded. */
/*    HTTP uses language tags within the Accept-Language and Content- */
/*    Language fields. */

/*    The syntax and registry of HTTP language tags is the same as that */
/*    defined by RFC 1766 [1]. In summary, a language tag is composed of 1 */
/*    or more parts: A primary language tag and a possibly empty series of */
/*    subtags: */

/*         language-tag  = primary-tag *( "-" subtag ) */
/*         primary-tag   = 1*8ALPHA */
/*         subtag        = 1*8ALPHA */

/*    White space is not allowed within the tag and all tags are case- */
/*    insensitive. The name space of language tags is administered by the */
/*    IANA. Example tags include: */

/*        en, en-US, en-cockney, i-cherokee, x-pig-latin */

/*    where any two-letter primary-tag is an ISO-639 language abbreviation */
/*    and any two-letter initial subtag is an ISO-3166 country code. (The */
/*    last three tags above are not registered tags; all but the last are */
/*    examples of tags which could be registered in future.) */

/* 3.11 Entity Tags */

/*    Entity tags are used for comparing two or more entities from the same */
/*    requested resource. HTTP/1.1 uses entity tags in the ETag (section */
/*    14.19), If-Match (section 14.24), If-None-Match (section 14.26), and */
/*    If-Range (section 14.27) header fields. The definition of how they */
/*    are used and compared as cache validators is in section 13.3.3. An */
/*    entity tag consists of an opaque quoted string, possibly prefixed by */
/*    a weakness indicator. */

/*       entity-tag = [ weak ] opaque-tag */
/*       weak       = "W/" */
/*       opaque-tag = quoted-string */

/*    A "strong entity tag" MAY be shared by two entities of a resource */
/*    only if they are equivalent by octet equality. */

/*    A "weak entity tag," indicated by the "W/" prefix, MAY be shared by */
/*    two entities of a resource only if the entities are equivalent and */
/*    could be substituted for each other with no significant change in */
/*    semantics. A weak entity tag can only be used for weak comparison. */

/*    An entity tag MUST be unique across all versions of all entities */
/*    associated with a particular resource. A given entity tag value MAY */
/*    be used for entities obtained by requests on different URIs. The use */
/*    of the same entity tag value in conjunction with entities obtained by */
/*    requests on different URIs does not imply the equivalence of those */
/*    entities. */

/* 3.12 Range Units */

/*    HTTP/1.1 allows a client to request that only part (a range of) the */
/*    response entity be included within the response. HTTP/1.1 uses range */
/*    units in the Range (section 14.35) and Content-Range (section 14.16) */
/*    header fields. An entity can be broken down into subranges according */
/*    to various structural units. */

/*       range-unit       = bytes-unit | other-range-unit */
/*       bytes-unit       = "bytes" */
/*       other-range-unit = token */

/*    The only range unit defined by HTTP/1.1 is "bytes". HTTP/1.1 */
/*    implementations MAY ignore ranges specified using other units. */

/*    HTTP/1.1 has been designed to allow implementations of applications */
/*    that do not depend on knowledge of ranges. */

/* 4 HTTP Message */

/* 4.1 Message Types */

/*    HTTP messages consist of requests from client to server and responses */
/*    from server to client. */

/*        HTTP-message   = Request | Response     ; HTTP/1.1 messages */

/*    Request (section 5) and Response (section 6) messages use the generic */
/*    message format of RFC 822 [9] for transferring entities (the payload */
/*    of the message). Both types of message consist of a start-line, zero */
/*    or more header fields (also known as "headers"), an empty line (i.e., */
/*    a line with nothing preceding the CRLF) indicating the end of the */
/*    header fields, and possibly a message-body. */

/*         generic-message = start-line */
/*                           *(message-header CRLF) */
/*                           CRLF */
/*                           [ message-body ] */
/*         start-line      = Request-Line | Status-Line */

/*    In the interest of robustness, servers SHOULD ignore any empty */
/*    line(s) received where a Request-Line is expected. In other words, if */
/*    the server is reading the protocol stream at the beginning of a */
/*    message and receives a CRLF first, it should ignore the CRLF. */

/*    Certain buggy HTTP/1.0 client implementations generate extra CRLF's */
/*    after a POST request. To restate what is explicitly forbidden by the */
/*    BNF, an HTTP/1.1 client MUST NOT preface or follow a request with an */
/*    extra CRLF. */

/* 4.2 Message Headers */

/*    HTTP header fields, which include general-header (section 4.5), */
/*    request-header (section 5.3), response-header (section 6.2), and */
/*    entity-header (section 7.1) fields, follow the same generic format as */
/*    that given in Section 3.1 of RFC 822 [9]. Each header field consists */
/*    of a name followed by a colon (":") and the field value. Field names */
/*    are case-insensitive. The field value MAY be preceded by any amount */
/*    of LWS, though a single SP is preferred. Header fields can be */
/*    extended over multiple lines by preceding each extra line with at */
/*    least one SP or HT. Applications ought to follow "common form", where */
/*    one is known or indicated, when generating HTTP constructs, since */
/*    there might exist some implementations that fail to accept anything */
/*    beyond the common forms. */

/*        message-header = field-name ":" [ field-value ] */
DECLARE_RULE(FIELD_NAME)
DECLARE_RULE(FIELD_VALUE)
RULE(MESSAGE_HEADER,
     ONE(CALL(FIELD_NAME))
     MANY(STRING(" ") || STRING("\t"))
     ONE(STRING(":"))
     MANY(STRING(" ") || STRING("\t"))
     OPTIONAL(CALL(FIELD_VALUE))
     ONE(STRING("\r\n")))

/*        field-name     = token */
RULE(FIELD_NAME,
     ONE(CALL(TOKEN)))

/*        field-value    = *( field-content | LWS ) */
DECLARE_RULE(FIELD_CONTENT)
RULE(FIELD_VALUE,
     MANY(CALL(FIELD_CONTENT)))
/*        field-content  = <the OCTETs making up the field-value */
/*                         and consisting of either *TEXT or combinations */
/*                         of token, separators, and quoted-string> */
RULE(FIELD_CONTENT,
     AT_LEAST_ONE(RANGE('\0', 9) || RANGE(14, 127)))

/*    The field-content does not include any leading or trailing LWS: */
/*    linear white space occurring before the first non-whitespace */
/*    character of the field-value or after the last non-whitespace */
/*    character of the field-value. Such leading or trailing LWS MAY be */
/*    removed without changing the semantics of the field value. Any LWS */
/*    that occurs between field-content MAY be replaced with a single SP */
/*    before interpreting the field value or forwarding the message */
/*    downstream. */

/*    The order in which header fields with differing field names are */
/*    received is not significant. However, it is "good practice" to send */
/*    general-header fields first, followed by request-header or response- */
/*    header fields, and ending with the entity-header fields. */

/*    Multiple message-header fields with the same field-name MAY be */
/*    present in a message if and only if the entire field-value for that */
/*    header field is defined as a comma-separated list [i.e., #(values)]. */
/*    It MUST be possible to combine the multiple header fields into one */
/*    "field-name: field-value" pair, without changing the semantics of the */
/*    message, by appending each subsequent field-value to the first, each */
/*    separated by a comma. The order in which header fields with the same */
/*    field-name are received is therefore significant to the */
/*    interpretation of the combined field value, and thus a proxy MUST NOT */
/*    change the order of these field values when a message is forwarded. */

/* 4.3 Message Body */

/*    The message-body (if any) of an HTTP message is used to carry the */
/*    entity-body associated with the request or response. The message-body */
/*    differs from the entity-body only when a transfer-coding has been */
/*    applied, as indicated by the Transfer-Encoding header field (section */
/*    14.41). */

/*        message-body = entity-body */
/*                     | <entity-body encoded as per Transfer-Encoding> */

/*    Transfer-Encoding MUST be used to indicate any transfer-codings */
/*    applied by an application to ensure safe and proper transfer of the */
/*    message. Transfer-Encoding is a property of the message, not of the */
/*    entity, and thus MAY be added or removed by any application along the */
/*    request/response chain. (However, section 3.6 places restrictions on */
/*    when certain transfer-codings may be used.) */

/*    The rules for when a message-body is allowed in a message differ for */
/*    requests and responses. */

/*    The presence of a message-body in a request is signaled by the */
/*    inclusion of a Content-Length or Transfer-Encoding header field in */
/*    the request's message-headers. A message-body MUST NOT be included in */
/*    a request if the specification of the request method (section 5.1.1) */
/*    does not allow sending an entity-body in requests. A server SHOULD */
/*    read and forward a message-body on any request; if the request method */
/*    does not include defined semantics for an entity-body, then the */
/*    message-body SHOULD be ignored when handling the request. */

/*    For response messages, whether or not a message-body is included with */
/*    a message is dependent on both the request method and the response */
/*    status code (section 6.1.1). All responses to the HEAD request method */
/*    MUST NOT include a message-body, even though the presence of entity- */
/*    header fields might lead one to believe they do. All 1xx */
/*    (informational), 204 (no content), and 304 (not modified) responses */
/*    MUST NOT include a message-body. All other responses do include a */
/*    message-body, although it MAY be of zero length. */

/* 4.4 Message Length */

/*    The transfer-length of a message is the length of the message-body as */
/*    it appears in the message; that is, after any transfer-codings have */
/*    been applied. When a message-body is included with a message, the */
/*    transfer-length of that body is determined by one of the following */
/*    (in order of precedence): */

/*    1.Any response message which "MUST NOT" include a message-body (such */
/*      as the 1xx, 204, and 304 responses and any response to a HEAD */
/*      request) is always terminated by the first empty line after the */
/*      header fields, regardless of the entity-header fields present in */
/*      the message. */

/*    2.If a Transfer-Encoding header field (section 14.41) is present and */
/*      has any value other than "identity", then the transfer-length is */
/*      defined by use of the "chunked" transfer-coding (section 3.6), */
/*      unless the message is terminated by closing the connection. */

/*    3.If a Content-Length header field (section 14.13) is present, its */
/*      decimal value in OCTETs represents both the entity-length and the */
/*      transfer-length. The Content-Length header field MUST NOT be sent */
/*      if these two lengths are different (i.e., if a Transfer-Encoding */
/*      header field is present). If a message is received with both a */
/*      Transfer-Encoding header field and a Content-Length header field, */
/*      the latter MUST be ignored. */

/*    4.If the message uses the media type "multipart/byteranges", and the */
/*      ransfer-length is not otherwise specified, then this self- */
/*      elimiting media type defines the transfer-length. This media type */
/*      UST NOT be used unless the sender knows that the recipient can arse */
/*      it; the presence in a request of a Range header with ultiple byte- */
/*      range specifiers from a 1.1 client implies that the lient can parse */
/*      multipart/byteranges responses. */

/*        A range header might be forwarded by a 1.0 proxy that does not */
/*        understand multipart/byteranges; in this case the server MUST */
/*        delimit the message using methods defined in items 1,3 or 5 of */
/*        this section. */

/*    5.By the server closing the connection. (Closing the connection */
/*      cannot be used to indicate the end of a request body, since that */
/*      would leave no possibility for the server to send back a response.) */

/*    For compatibility with HTTP/1.0 applications, HTTP/1.1 requests */
/*    containing a message-body MUST include a valid Content-Length header */
/*    field unless the server is known to be HTTP/1.1 compliant. If a */
/*    request contains a message-body and a Content-Length is not given, */
/*    the server SHOULD respond with 400 (bad request) if it cannot */
/*    determine the length of the message, or with 411 (length required) if */
/*    it wishes to insist on receiving a valid Content-Length. */

/*    All HTTP/1.1 applications that receive entities MUST accept the */
/*    "chunked" transfer-coding (section 3.6), thus allowing this mechanism */
/*    to be used for messages when the message length cannot be determined */
/*    in advance. */

/*    Messages MUST NOT include both a Content-Length header field and a */
/*    non-identity transfer-coding. If the message does include a non- */
/*    identity transfer-coding, the Content-Length MUST be ignored. */

/*    When a Content-Length is given in a message where a message-body is */
/*    allowed, its field value MUST exactly match the number of OCTETs in */
/*    the message-body. HTTP/1.1 user agents MUST notify the user when an */
/*    invalid length is received and detected. */

/* 4.5 General Header Fields */

/*    There are a few header fields which have general applicability for */
/*    both request and response messages, but which do not apply to the */
/*    entity being transferred. These header fields apply only to the */
/*    message being transmitted. */

/*        general-header = Cache-Control            ; Section 14.9 */
/*                       | Connection               ; Section 14.10 */
/*                       | Date                     ; Section 14.18 */
/*                       | Pragma                   ; Section 14.32 */
/*                       | Trailer                  ; Section 14.40 */
/*                       | Transfer-Encoding        ; Section 14.41 */
/*                       | Upgrade                  ; Section 14.42 */
/*                       | Via                      ; Section 14.45 */
/*                       | Warning                  ; Section 14.46 */

/*    General-header field names can be extended reliably only in */
/*    combination with a change in the protocol version. However, new or */
/*    experimental header fields may be given the semantics of general */
/*    header fields if all parties in the communication recognize them to */
/*    be general-header fields. Unrecognized header fields are treated as */
/*    entity-header fields. */

/* 5 Request */

/*    A request message from a client to a server includes, within the */
/*    first line of that message, the method to be applied to the resource, */
/*    the identifier of the resource, and the protocol version in use. */

/*         Request       = Request-Line              ; Section 5.1 */
/*                         *(( general-header        ; Section 4.5 */
/*                          | request-header         ; Section 5.3 */
/*                          | entity-header ) CRLF)  ; Section 7.1 */
/*                         CRLF */
/*                         [ message-body ]          ; Section 4.3 */
DECLARE_RULE(REQUEST_LINE)
RULE(REQUEST,
     ONE(CALL(REQUEST_LINE))
     MANY(CALL(MESSAGE_HEADER)) /* TODO : Remove the shortcut on headers */
     ONE(CALL(CRLF)))

/* 5.1 Request-Line */

/*    The Request-Line begins with a method token, followed by the */
/*    Request-URI and the protocol version, and ending with CRLF. The */
/*    elements are separated by SP characters. No CR or LF is allowed */
/*    except in the final CRLF sequence. */

/*         Request-Line   = Method SP Request-URI SP HTTP-Version CRLF */
DECLARE_RULE(METHOD)
DECLARE_RULE(REQUEST_URI)
RULE(REQUEST_LINE,
     ONE(CALL(METHOD))
     ONE(CALL(SP))
     ONE(CALL(REQUEST_URI))
     ONE(CALL(SP))
     ONE(CALL(HTTP_VERSION))
     ONE(CALL(CRLF)))

/* 5.1.1 Method */

/*    The Method  token indicates the method to be performed on the */
/*    resource identified by the Request-URI. The method is case-sensitive. */

/*        Method         = "OPTIONS"                ; Section 9.2 */
/*                       | "GET"                    ; Section 9.3 */
/*                       | "HEAD"                   ; Section 9.4 */
/*                       | "POST"                   ; Section 9.5 */
/*                       | "PUT"                    ; Section 9.6 */
/*                       | "DELETE"                 ; Section 9.7 */
/*                       | "TRACE"                  ; Section 9.8 */
/*                       | "CONNECT"                ; Section 9.9 */
/*                       | extension-method */
/*        extension-method = token */
RULE(METHOD,
     ONE(CALL(TOKEN)))

/*    The list of methods allowed by a resource can be specified in an */
/*    Allow header field (section 14.7). The return code of the response */
/*    always notifies the client whether a method is currently allowed on a */
/*    resource, since the set of allowed methods can change dynamically. An */
/*    origin server SHOULD return the status code 405 (Method Not Allowed) */
/*    if the method is known by the origin server but not allowed for the */
/*    requested resource, and 501 (Not Implemented) if the method is */
/*    unrecognized or not implemented by the origin server. The methods GET */
/*    and HEAD MUST be supported by all general-purpose servers. All other */
/*    methods are OPTIONAL; however, if the above methods are implemented, */
/*    they MUST be implemented with the same semantics as those specified */
/*    in section 9. */

/* 5.1.2 Request-URI */

/*    The Request-URI is a Uniform Resource Identifier (section 3.2) and */
/*    identifies the resource upon which to apply the request. */

/*        Request-URI    = "*" | absoluteURI | abs_path | authority */
RULE(REQUEST_URI,
     ONE(STRING("*")
         || CALL(ABSOLUTE_URI)
         || CALL(ABS_PATH)
         || CALL(AUTHORITY)))

/*    The four options for Request-URI are dependent on the nature of the */
/*    request. The asterisk "*" means that the request does not apply to a */
/*    particular resource, but to the server itself, and is only allowed */
/*    when the method used does not necessarily apply to a resource. One */
/*    example would be */

/*        OPTIONS * HTTP/1.1 */

/*    The absoluteURI form is REQUIRED when the request is being made to a */
/*    proxy. The proxy is requested to forward the request or service it */
/*    from a valid cache, and return the response. Note that the proxy MAY */
/*    forward the request on to another proxy or directly to the server */
/*    specified by the absoluteURI. In order to avoid request loops, a */
/*    proxy MUST be able to recognize all of its server names, including */
/*    any aliases, local variations, and the numeric IP address. An example */
/*    Request-Line would be: */

/*        GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1 */

/*    To allow for transition to absoluteURIs in all requests in future */
/*    versions of HTTP, all HTTP/1.1 servers MUST accept the absoluteURI */
/*    form in requests, even though HTTP/1.1 clients will only generate */
/*    them in requests to proxies. */

/*    The authority form is only used by the CONNECT method (section 9.9). */

/*    The most common form of Request-URI is that used to identify a */
/*    resource on an origin server or gateway. In this case the absolute */
/*    path of the URI MUST be transmitted (see section 3.2.1, abs_path) as */
/*    the Request-URI, and the network location of the URI (authority) MUST */
/*    be transmitted in a Host header field. For example, a client wishing */
/*    to retrieve the resource above directly from the origin server would */
/*    create a TCP connection to port 80 of the host "www.w3.org" and send */
/*    the lines: */

/*        GET /pub/WWW/TheProject.html HTTP/1.1 */
/*        Host: www.w3.org */

/*    followed by the remainder of the Request. Note that the absolute path */
/*    cannot be empty; if none is present in the original URI, it MUST be */
/*    given as "/" (the server root). */

/*    The Request-URI is transmitted in the format specified in section */
/*    3.2.1. If the Request-URI is encoded using the "% HEX HEX" encoding */
/*    [42], the origin server MUST decode the Request-URI in order to */
/*    properly interpret the request. Servers SHOULD respond to invalid */
/*    Request-URIs with an appropriate status code. */

/*    A transparent proxy MUST NOT rewrite the "abs_path" part of the */
/*    received Request-URI when forwarding it to the next inbound server, */
/*    except as noted above to replace a null abs_path with "/". */

/*       Note: The "no rewrite" rule prevents the proxy from changing the */
/*       meaning of the request when the origin server is improperly using */
/*       a non-reserved URI character for a reserved purpose.  Implementors */
/*       should be aware that some pre-HTTP/1.1 proxies have been known to */
/*       rewrite the Request-URI. */

/* 5.2 The Resource Identified by a Request */

/*    The exact resource identified by an Internet request is determined by */
/*    examining both the Request-URI and the Host header field. */

/*    An origin server that does not allow resources to differ by the */
/*    requested host MAY ignore the Host header field value when */
/*    determining the resource identified by an HTTP/1.1 request. (But see */
/*    section 19.6.1.1 for other requirements on Host support in HTTP/1.1.) */

/*    An origin server that does differentiate resources based on the host */
/*    requested (sometimes referred to as virtual hosts or vanity host */
/*    names) MUST use the following rules for determining the requested */
/*    resource on an HTTP/1.1 request: */

/*    1. If Request-URI is an absoluteURI, the host is part of the */
/*      Request-URI. Any Host header field value in the request MUST be */
/*      ignored. */

/*    2. If the Request-URI is not an absoluteURI, and the request includes */
/*      a Host header field, the host is determined by the Host header */
/*      field value. */

/*    3. If the host as determined by rule 1 or 2 is not a valid host on */
/*      the server, the response MUST be a 400 (Bad Request) error message. */

/*    Recipients of an HTTP/1.0 request that lacks a Host header field MAY */
/*    attempt to use heuristics (e.g., examination of the URI path for */
/*    something unique to a particular host) in order to determine what */
/*    exact resource is being requested. */

/* 5.3 Request Header Fields */

/*    The request-header fields allow the client to pass additional */
/*    information about the request, and about the client itself, to the */
/*    server. These fields act as request modifiers, with semantics */
/*    equivalent to the parameters on a programming language method */
/*    invocation. */

/*        request-header = Accept                   ; Section 14.1 */
/*                       | Accept-Charset           ; Section 14.2 */
/*                       | Accept-Encoding          ; Section 14.3 */
/*                       | Accept-Language          ; Section 14.4 */
/*                       | Authorization            ; Section 14.8 */
/*                       | Expect                   ; Section 14.20 */
/*                       | From                     ; Section 14.22 */
/*                       | Host                     ; Section 14.23 */
/*                       | If-Match                 ; Section 14.24 */
/*                       | If-Modified-Since        ; Section 14.25 */
/*                       | If-None-Match            ; Section 14.26 */
/*                       | If-Range                 ; Section 14.27 */
/*                       | If-Unmodified-Since      ; Section 14.28 */
/*                       | Max-Forwards             ; Section 14.31 */
/*                       | Proxy-Authorization      ; Section 14.34 */
/*                       | Range                    ; Section 14.35 */
/*                       | Referer                  ; Section 14.36 */
/*                       | TE                       ; Section 14.39 */
/*                       | User-Agent               ; Section 14.43 */

/*    Request-header field names can be extended reliably only in */
/*    combination with a change in the protocol version. However, new or */
/*    experimental header fields MAY be given the semantics of request- */
/*    header fields if all parties in the communication recognize them to */
/*    be request-header fields. Unrecognized header fields are treated as */
/*    entity-header fields. */

struct sized_string
{
    char   *ptr;
    size_t len;
};

struct http_header
{
    char           *name;
    char           *value;
    UT_hash_handle hh;
};

struct http_request
{
    struct sized_string method;
    struct sized_string request_uri;
    struct sized_string http_version;
    struct sized_string host;
    struct http_header  *headers;
    int                 complete;
};

void output(char *rule, char *ptr, int len, void *user_data)
{
    struct http_request *req;
    static struct sized_string field_name = {NULL, 0};
    static struct sized_string field_value = {NULL, 0};
    struct http_header *header;

    req = (struct http_request*)user_data;
#define retrieve_rule(rulename, field)          \
    if ((void*)rule == (void*)#rulename)        \
    {                                           \
        req->field.ptr = ptr;                   \
        req->field.len = len;                   \
    }
    if ((void*)rule == (void*)"REQUEST")
    {
        req->complete = 1;
    }
    retrieve_rule(METHOD, method);
    retrieve_rule(REQUEST_URI, request_uri);
    retrieve_rule(HTTP_VERSION, http_version);
#undef retrieve_rule
    if ((void*)rule == (void*)"MESSAGE_HEADER")
    {
        field_name.ptr[field_name.len] = '\0';
        if (field_value.ptr != NULL)
            field_value.ptr[field_value.len] = '\0';
        if (strcmp(field_name.ptr, "Host") == 0)
            req->host = field_value;
        header = (struct http_header*)malloc(sizeof(*header));
        header->name = field_name.ptr;
        header->value = field_value.ptr;
        HASH_ADD_KEYPTR(hh, req->headers, header->name, field_name.len, header);
        field_name.ptr = field_value.ptr = NULL;
    }
    else if ((void*)rule == (void*)"FIELD_NAME")
    {
        field_name.ptr = ptr;
        field_name.len = len;
    }
    else if ((void*)rule == (void*)"FIELD_VALUE")
    {
        field_value.ptr = ptr;
        field_value.len = len;
    }
    return;
    printf("%s : ", rule);
    while (len > 0)
    {
        chrdump(*ptr++);
        len -= 1;
    }
    printf("\n");
}

struct http_request *parse(char *str)
{
    struct http_request *http_request;
    struct http_header *header, *tmp;

    http_request = (struct http_request *)calloc(1, sizeof(*http_request));
    http_request->headers = NULL;
    rule_REQUEST(&str, output, http_request);
    if (http_request->complete)
    {
#define terminate(rule)                                             \
        if (http_request->rule.ptr != NULL)                         \
            http_request->rule.ptr[http_request->rule.len] = '\0';
        terminate(method);
        terminate(request_uri);
        terminate(http_version);
#undef terminate
    }
    else
    {
        HASH_ITER(hh, http_request->headers, header, tmp)
        {
            free(header->name);
            if (header->value != NULL)
                free(header->value);
        }
        free(http_request);
    }
    return http_request;
}

int main(int ac __attribute__((unused)), char **av)
{
    struct http_header *http_header;
    struct http_request *http_request;
    char *req = strdup(av[1]);

    http_request = parse(req);
    if (http_request->complete)
    {
#define terminate(rule) if (http_request->rule.ptr != NULL) http_request->rule.ptr[http_request->rule.len] = '\0';
        terminate(method);
        terminate(request_uri);
        terminate(http_version);
#undef terminate
        printf("Method  : %s\n", http_request->method.ptr);
        printf("URI     : %s\n", http_request->request_uri.ptr);
        printf("Version : %s\n", http_request->http_version.ptr);
        for (http_header = http_request->headers;
             http_header != NULL;
             http_header = (struct http_header *)http_header->hh.next)
        {
            printf(" | %s => %s\n", http_header->name, http_header->value);
        }
    }
    else
    {
        printf("Invalid request\n");
    }
    return EXIT_SUCCESS;
}
