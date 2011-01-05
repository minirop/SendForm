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
#ifndef SEND_FORM_WITH_QT
#define SEND_FORM_WITH_QT

#include <QByteArray>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>
#include <QUrl>

////////////////////////////////////////////////////////////
/// SendForm can send files and fields (html's &lt;input&gt;) to a website.
////////////////////////////////////////////////////////////
class SendForm
{
public:
	////////////////////////////////////////////////////////////
	/// Constructor
	///
	/// \param destination : url that will received the form
	///
	////////////////////////////////////////////////////////////
	SendForm( const QUrl & destination );
	
	////////////////////////////////////////////////////////////
	/// add a field to the form
	///
	/// \param name : the name of the field
	/// \param value : the value of the field
	///
	////////////////////////////////////////////////////////////
	void addField( const QString & name, const QString & value );
	
	////////////////////////////////////////////////////////////
	/// add several fields at once
	///
	/// \param names : the names of the field
	/// \param values : the values of the field
	///
	/// \remarks if names and values doesn't have the same number of elements, only min(names.count(), values.count()) elements will be added
	///
	/// \see addField
	///
	////////////////////////////////////////////////////////////
	void addFields( const QStringList & names, const QStringList & values );
	
	////////////////////////////////////////////////////////////
	/// remove all fields (except the "file" ones) of the form
	///
	/// \see clearFiles
	///
	////////////////////////////////////////////////////////////
	void clearFields();
	
	////////////////////////////////////////////////////////////
	/// add a field "file" to the form
	///
	/// \param name : the name of the field
	/// \param file : the file path
	///
	////////////////////////////////////////////////////////////
	void addFile( const QString & name, const QString & file );
	
	////////////////////////////////////////////////////////////
	/// add several files at once
	///
	/// \param names : the names of the field
	/// \param files : the files path
	///
	/// \remarks if names and files doesn't have the same number of elements, only min(names.count(), files.count()) elements will be added
	///
	/// \see addFile
	///
	////////////////////////////////////////////////////////////
	void addFiles( const QStringList & names, const QStringList & files );
	
	////////////////////////////////////////////////////////////
	/// remove all the files of the form
	///
	////////////////////////////////////////////////////////////
	void clearFiles();
	
	////////////////////////////////////////////////////////////
	/// remove all the fields and all the files of the form
	///
	/// \see clearFiles
	/// \see clearFields
	///
	////////////////////////////////////////////////////////////
	void clear();
	
	////////////////////////////////////////////////////////////
	/// set a header
	///
	/// \param headerName : the name of the header (i.e. "Content-type", "Accept", ...)
	/// \param headerValue : the value of the field
	///
	////////////////////////////////////////////////////////////
	void setHeader( const QByteArray & headerName, const QByteArray & headerValue );
	
	////////////////////////////////////////////////////////////
	/// set the referer's url
	///
	/// \param url : the url you come from
	///
	////////////////////////////////////////////////////////////
	void setReferer( const QString & url );
	
	////////////////////////////////////////////////////////////
	/// set the user-agent to "Mozilla/5.0"
	///
	////////////////////////////////////////////////////////////
	void setDefaultUserAgent();
	
	////////////////////////////////////////////////////////////
	/// force the form to be send as "multipart/form-data" instead of "x-www-form-urlencoded"
	///
	/// \see removeMultipart
	///
	////////////////////////////////////////////////////////////
	void forceMultipart();
	
	////////////////////////////////////////////////////////////
	/// force the form to be send as "x-www-form-urlencoded" instead of "multipart/form-data" if you called "forceMultipart" before
	///
	/// \see forceMultipart
	///
	/// \remarks ignored if you added a file
	///
	////////////////////////////////////////////////////////////
	void removeMultipart();
	
	////////////////////////////////////////////////////////////
	/// send the form
	///
	/// \param manager : a valid QNetworkAccessManager that will be used to
	///
	/// \return return the QNetworkReply corresponding to the request (or 0 if it's an invalid request)
	///
	////////////////////////////////////////////////////////////
	QNetworkReply * post( QNetworkAccessManager * manager );

private:
	QNetworkRequest m_request;
	QUrl m_destination;
	QHash< QString, QString > m_files;
	QHash< QString, QString > m_fields;
	bool m_forcemultipart;
	
	static void init();
	static bool m_initialized;
	static QHash< QString, QByteArray > mimeTypes;
};

#endif
