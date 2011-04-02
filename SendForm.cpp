/******************************************************************************
* Copyright (C) 2010, Roper Alexander
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
* 
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
******************************************************************************/
#include "SendForm.h"

#include <QFile>
#include <QFileInfo>
#include <QHashIterator>
#include <ctime>

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
SendForm::SendForm( const QUrl & destination )
{
	if( !m_initialized ) init();
	
	m_request.setUrl( destination );
	setReferer( destination.host() );
	
	m_forcemultipart = false;
}

////////////////////////////////////////////////////////////
/// add a field "file" to the form
////////////////////////////////////////////////////////////
void SendForm::addFile( const QString & name, const QString & file )
{
	m_files[ name ] = file;
}

////////////////////////////////////////////////////////////
/// add several files at once
////////////////////////////////////////////////////////////
void SendForm::addFiles( const QStringList & names, const QStringList & files )
{
	int numberOfFiles = qMin( names.count(), files.count() );
	for(int i = 0;i < numberOfFiles;i++)
	{
		addFile( names[i], files[i] );
	}
}

////////////////////////////////////////////////////////////
/// add a field to the form
////////////////////////////////////////////////////////////
void SendForm::addField( const QString & name, const QString & value )
{
	m_fields[ name ] = value;
}

////////////////////////////////////////////////////////////
/// add several fields at once
////////////////////////////////////////////////////////////
void SendForm::addFields( const QStringList & names, const QStringList & values )
{
	int numberOfFields = qMin( names.count(), values.count() );
	for(int i = 0;i < numberOfFields;i++)
	{
		addField( names[i], values[i] );
	}
}

////////////////////////////////////////////////////////////
/// set the referer's url
////////////////////////////////////////////////////////////
void SendForm::setReferer( const QString & url )
{
	m_request.setRawHeader( "Referer", url.toAscii() );
}

////////////////////////////////////////////////////////////
/// set a header
////////////////////////////////////////////////////////////
void SendForm::setHeader( const QByteArray & headerName, const QByteArray & headerValue )
{
	m_request.setRawHeader( headerName, headerValue );
}

////////////////////////////////////////////////////////////
/// set the user-agent to "Mozilla/5.0"
////////////////////////////////////////////////////////////
void SendForm::setDefaultUserAgent()
{
	setHeader( "User-Agent", "Mozilla/5.0" );
}

////////////////////////////////////////////////////////////
/// remove all the fields and all the files of the form
////////////////////////////////////////////////////////////
void SendForm::clear()
{
	clearFiles();
	clearFields();
}

////////////////////////////////////////////////////////////
/// remove all the files of the form
////////////////////////////////////////////////////////////
void SendForm::clearFiles()
{
	m_files.clear();
}

////////////////////////////////////////////////////////////
/// remove all fields (except the "file" ones) of the form
////////////////////////////////////////////////////////////
void SendForm::clearFields()
{
	m_fields.clear();
}

////////////////////////////////////////////////////////////
/// send the form
////////////////////////////////////////////////////////////
QNetworkReply * SendForm::post( QNetworkAccessManager * manager )
{
	QByteArray boundary = generateBoundary();
	
	QByteArray temp_data;
	
	QHashIterator< QString, QString > i( m_files );
	while( i.hasNext() )
	{
		i.next();
		QFile file( i.value() );
		if( file.open( QIODevice::ReadOnly ) )
		{
			QFileInfo fi( file );
			QString ext = fi.suffix();
			
			QByteArray mime = "text/plain";
			if( mimeTypes.contains( ext ) )
			{
				mime = mimeTypes[ext];
			}
			
			QByteArray aadata = "--" + boundary;
			aadata += "\r\nContent-Disposition: form-data; name=\"" + i.key().toAscii() + "\"; filename=\"" + fi.fileName().toAscii() + "\";\r\n";
			aadata += "Content-Type: " + mime + "\r\n\r\n" + file.readAll() + "\r\n";
			file.close();
			
			temp_data += aadata;
		}
	}
	
	if( m_files.count() || m_forcemultipart )
	{
		QHashIterator< QString, QString > i( m_fields );
		while( i.hasNext() )
		{
			i.next();
			
			QByteArray aadata = "--" + boundary;
			aadata += "\r\nContent-Disposition: form-data; name=\"" + i.key().toAscii() + "\"\r\n\r\n";
			aadata += i.value().toAscii() + "\r\n";
			
			temp_data += aadata;
		}
		
		m_request.setRawHeader( "Content-Type", "multipart/form-data; boundary=" + boundary );
		temp_data += "--" + boundary + "--\r\n";
	}
	else
	{
		QHashIterator< QString, QString > i( m_fields );
		while( i.hasNext() )
		{
			i.next();
			
			temp_data += "&" + i.key().toAscii() + "=" + i.value().toAscii();
		}
		
		temp_data.remove(0, 1);
	}
	
	m_request.setRawHeader( "Content-Length", QByteArray::number( temp_data.size() ) );
	
	return manager->post( m_request, temp_data );
}

////////////////////////////////////////////////////////////
/// force the form to be send as "multipart/form-data" instead of "x-www-form-urlencoded"
////////////////////////////////////////////////////////////
void SendForm::forceMultipart()
{
	m_forcemultipart = true;
}

////////////////////////////////////////////////////////////
/// force the form to be send as "x-www-form-urlencoded" instead of "multipart/form-data" if you called "forceMultipart" before
////////////////////////////////////////////////////////////
void SendForm::removeMultipart()
{
	m_forcemultipart = false;
}

QByteArray SendForm::generateBoundary()
{
	QByteArray bchars = "0123456789AZERTYUIOPQSDFGHJKLMWXCVBNazertyuiopqsdfghjklmwxcvbn'()+_,-./:=?";
	QByteArray boundary;
	
	for(int i = 0;i < 60;i++) // boundary can be only between 1 and 70 characters (I chose 60 arbitrarily).
	{
		boundary += bchars[qrand()%bchars.length()];
	}
	
	return boundary;
}

bool SendForm::m_initialized = false;
QHash< QString, QByteArray > SendForm::mimeTypes;

void SendForm::init()
{
	qsrand(time(0));
	
	mimeTypes["323"] = "text/h323";
	mimeTypes["acx"] = "application/internet-property-stream";
	mimeTypes["ai"] = "application/postscript";
	mimeTypes["aif"] = "audio/x-aiff";
	mimeTypes["aifc"] = "audio/x-aiff";
	mimeTypes["aiff"] = "audio/x-aiff";
	mimeTypes["asf"] = "video/x-ms-asf";
	mimeTypes["asr"] = "video/x-ms-asf";
	mimeTypes["asx"] = "video/x-ms-asf";
	mimeTypes["au"] = "audio/basic";
	mimeTypes["avi"] = "video/x-msvideo";
	mimeTypes["axs"] = "application/olescript";
	mimeTypes["bas"] = "text/plain";
	mimeTypes["bcpio"] = "application/x-bcpio";
	mimeTypes["bin"] = "application/octet-stream";
	mimeTypes["bmp"] = "image/bmp";
	mimeTypes["c"] = "text/plain";
	mimeTypes["cat"] = "application/vnd.ms-pkiseccat";
	mimeTypes["cdf"] = "application/x-cdf";
	mimeTypes["cer"] = "application/x-x509-ca-cert";
	mimeTypes["class"] = "application/octet-stream";
	mimeTypes["clp"] = "application/x-msclip";
	mimeTypes["cmx"] = "image/x-cmx";
	mimeTypes["cod"] = "image/cis-cod";
	mimeTypes["cpio"] = "application/x-cpio";
	mimeTypes["crd"] = "application/x-mscardfile";
	mimeTypes["crl"] = "application/pkix-crl";
	mimeTypes["crt"] = "application/x-x509-ca-cert";
	mimeTypes["csh"] = "application/x-csh";
	mimeTypes["css"] = "text/css";
	mimeTypes["dcr"] = "application/x-director";
	mimeTypes["der"] = "application/x-x509-ca-cert";
	mimeTypes["dir"] = "application/x-director";
	mimeTypes["dll"] = "application/x-msdownload";
	mimeTypes["dms"] = "application/octet-stream";
	mimeTypes["doc"] = "application/msword";
	mimeTypes["dot"] = "application/msword";
	mimeTypes["dvi"] = "application/x-dvi";
	mimeTypes["dxr"] = "application/x-director";
	mimeTypes["eps"] = "application/postscript";
	mimeTypes["etx"] = "text/x-setext";
	mimeTypes["evy"] = "application/envoy";
	mimeTypes["exe"] = "application/octet-stream";
	mimeTypes["fif"] = "application/fractals";
	mimeTypes["flr"] = "x-world/x-vrml";
	mimeTypes["gif"] = "image/gif";
	mimeTypes["gtar"] = "application/x-gtar";
	mimeTypes["gz"] = "application/x-gzip";
	mimeTypes["h"] = "text/plain";
	mimeTypes["hdf"] = "application/x-hdf";
	mimeTypes["hlp"] = "application/winhlp";
	mimeTypes["hqx"] = "application/mac-binhex40";
	mimeTypes["hta"] = "application/hta";
	mimeTypes["htc"] = "text/x-component";
	mimeTypes["htm"] = "text/html";
	mimeTypes["html"] = "text/html";
	mimeTypes["htt"] = "text/webviewhtml";
	mimeTypes["ico"] = "image/x-icon";
	mimeTypes["ief"] = "image/ief";
	mimeTypes["iii"] = "application/x-iphone";
	mimeTypes["ins"] = "application/x-internet-signup";
	mimeTypes["isp"] = "application/x-internet-signup";
	mimeTypes["jfif"] = "image/pipeg";
	mimeTypes["jpe"] = "image/jpeg";
	mimeTypes["jpeg"] = "image/jpeg";
	mimeTypes["jpg"] = "image/jpeg";
	mimeTypes["js"] = "application/x-javascript";
	mimeTypes["latex"] = "application/x-latex";
	mimeTypes["lha"] = "application/octet-stream";
	mimeTypes["lsf"] = "video/x-la-asf";
	mimeTypes["lsx"] = "video/x-la-asf";
	mimeTypes["lzh"] = "application/octet-stream";
	mimeTypes["m13"] = "application/x-msmediaview";
	mimeTypes["m14"] = "application/x-msmediaview";
	mimeTypes["m3u"] = "audio/x-mpegurl";
	mimeTypes["man"] = "application/x-troff-man";
	mimeTypes["mdb"] = "application/x-msaccess";
	mimeTypes["me"] = "application/x-troff-me";
	mimeTypes["mht"] = "message/rfc822";
	mimeTypes["mhtml"] = "message/rfc822";
	mimeTypes["mid"] = "audio/mid";
	mimeTypes["mny"] = "application/x-msmoney";
	mimeTypes["mov"] = "video/quicktime";
	mimeTypes["movie"] = "video/x-sgi-movie";
	mimeTypes["mp2"] = "video/mpeg";
	mimeTypes["mp3"] = "audio/mpeg";
	mimeTypes["mpa"] = "video/mpeg";
	mimeTypes["mpe"] = "video/mpeg";
	mimeTypes["mpeg"] = "video/mpeg";
	mimeTypes["mpg"] = "video/mpeg";
	mimeTypes["mpp"] = "application/vnd.ms-project";
	mimeTypes["mpv2"] = "video/mpeg";
	mimeTypes["ms"] = "application/x-troff-ms";
	mimeTypes["mvb"] = "application/x-msmediaview";
	mimeTypes["nws"] = "message/rfc822";
	mimeTypes["oda"] = "application/oda";
	mimeTypes["p10"] = "application/pkcs10";
	mimeTypes["p12"] = "application/x-pkcs12";
	mimeTypes["p7b"] = "application/x-pkcs7-certificates";
	mimeTypes["p7c"] = "application/x-pkcs7-mime";
	mimeTypes["p7m"] = "application/x-pkcs7-mime";
	mimeTypes["p7r"] = "application/x-pkcs7-certreqresp";
	mimeTypes["p7s"] = "application/x-pkcs7-signature";
	mimeTypes["pbm"] = "image/x-portable-bitmap";
	mimeTypes["pdf"] = "application/pdf";
	mimeTypes["pfx"] = "application/x-pkcs12";
	mimeTypes["pgm"] = "image/x-portable-graymap";
	mimeTypes["pko"] = "application/ynd.ms-pkipko";
	mimeTypes["pma"] = "application/x-perfmon";
	mimeTypes["pmc"] = "application/x-perfmon";
	mimeTypes["pml"] = "application/x-perfmon";
	mimeTypes["pmr"] = "application/x-perfmon";
	mimeTypes["pmw"] = "application/x-perfmon";
	mimeTypes["png"] = "image/png";
	mimeTypes["pnm"] = "image/x-portable-anymap";
	mimeTypes["pot"] = "application/vnd.ms-powerpoint";
	mimeTypes["ppm"] = "image/x-portable-pixmap";
	mimeTypes["pps"] = "application/vnd.ms-powerpoint";
	mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
	mimeTypes["prf"] = "application/pics-rules";
	mimeTypes["ps"] = "application/postscript";
	mimeTypes["pub"] = "application/x-mspublisher";
	mimeTypes["qt"] = "video/quicktime";
	mimeTypes["ra"] = "audio/x-pn-realaudio";
	mimeTypes["ram"] = "audio/x-pn-realaudio";
	mimeTypes["ras"] = "image/x-cmu-raster";
	mimeTypes["rgb"] = "image/x-rgb";
	mimeTypes["rmi"] = "audio/mid";
	mimeTypes["roff"] = "application/x-troff";
	mimeTypes["rtf"] = "application/rtf";
	mimeTypes["rtx"] = "text/richtext";
	mimeTypes["scd"] = "application/x-msschedule";
	mimeTypes["sct"] = "text/scriptlet";
	mimeTypes["setpay"] = "application/set-payment-initiation";
	mimeTypes["setreg"] = "application/set-registration-initiation";
	mimeTypes["sh"] = "application/x-sh";
	mimeTypes["shar"] = "application/x-shar";
	mimeTypes["sit"] = "application/x-stuffit";
	mimeTypes["snd"] = "audio/basic";
	mimeTypes["spc"] = "application/x-pkcs7-certificates";
	mimeTypes["spl"] = "application/futuresplash";
	mimeTypes["src"] = "application/x-wais-source";
	mimeTypes["sst"] = "application/vnd.ms-pkicertstore";
	mimeTypes["stl"] = "application/vnd.ms-pkistl";
	mimeTypes["stm"] = "text/html";
	mimeTypes["svg"] = "image/svg+xml";
	mimeTypes["sv4cpio"] = "application/x-sv4cpio";
	mimeTypes["sv4crc"] = "application/x-sv4crc";
	mimeTypes["swf"] = "application/x-shockwave-flash";
	mimeTypes["t"] = "application/x-troff";
	mimeTypes["tar"] = "application/x-tar";
	mimeTypes["tcl"] = "application/x-tcl";
	mimeTypes["tex"] = "application/x-tex";
	mimeTypes["texi"] = "application/x-texinfo";
	mimeTypes["texinfo"] = "application/x-texinfo";
	mimeTypes["tgz"] = "application/x-compressed";
	mimeTypes["tif"] = "image/tiff";
	mimeTypes["tiff"] = "image/tiff";
	mimeTypes["tr"] = "application/x-troff";
	mimeTypes["trm"] = "application/x-msterminal";
	mimeTypes["tsv"] = "text/tab-separated-values";
	mimeTypes["txt"] = "text/plain";
	mimeTypes["uls"] = "text/iuls";
	mimeTypes["ustar"] = "application/x-ustar";
	mimeTypes["vcf"] = "text/x-vcard";
	mimeTypes["vrml"] = "x-world/x-vrml";
	mimeTypes["wav"] = "audio/x-wav";
	mimeTypes["wcm"] = "application/vnd.ms-works";
	mimeTypes["wdb"] = "application/vnd.ms-works";
	mimeTypes["wks"] = "application/vnd.ms-works";
	mimeTypes["wmf"] = "application/x-msmetafile";
	mimeTypes["wps"] = "application/vnd.ms-works";
	mimeTypes["wri"] = "application/x-mswrite";
	mimeTypes["wrl"] = "x-world/x-vrml";
	mimeTypes["wrz"] = "x-world/x-vrml";
	mimeTypes["xaf"] = "x-world/x-vrml";
	mimeTypes["xbm"] = "image/x-xbitmap";
	mimeTypes["xla"] = "application/vnd.ms-excel";
	mimeTypes["xlc"] = "application/vnd.ms-excel";
	mimeTypes["xlm"] = "application/vnd.ms-excel";
	mimeTypes["xls"] = "application/vnd.ms-excel";
	mimeTypes["xlt"] = "application/vnd.ms-excel";
	mimeTypes["xlw"] = "application/vnd.ms-excel";
	mimeTypes["xof"] = "x-world/x-vrml";
	mimeTypes["xpm"] = "image/x-xpixmap";
	mimeTypes["xwd"] = "image/x-xwindowdump";
	mimeTypes["z"] = "application/x-compress";
	mimeTypes["zip"] = "application/zip";
}
