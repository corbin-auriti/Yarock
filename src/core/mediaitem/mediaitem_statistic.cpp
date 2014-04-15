/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
*                                                                                       *
*  This program is free software; you can redistribute it and/or modify it under        *
*  the terms of the GNU General Public License as published by the Free Software        *
*  Foundation; either version 2 of the License, or (at your option) any later           *
*  version.                                                                             *
*                                                                                       *
*  This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
*  PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                       *
*  You should have received a copy of the GNU General Public License along with         *
*  this program.  If not, see <http://www.gnu.org/licenses/>.                           *
*****************************************************************************************/

#include "mediaitem.h"

#include <QString>
#include <cmath>

// Taglib
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/popularimeterframe.h>
#include <taglib/textidentificationframe.h>

#include <taglib/xiphcomment.h>

#include <taglib/apetag.h>

#include <taglib/asftag.h>

#include <aifffile.h>
#include <asffile.h>
#include <flacfile.h>
#include <mp4file.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <speexfile.h>
#include <trueaudiofile.h>
#include <vorbisfile.h>
#include <wavfile.h>
#include <wavpackfile.h>

const TagLib::ByteVector TXXX_Frame = "TXXX";
const TagLib::ByteVector POPM_Frame = "POPM";

/*
********************************************************************************
*                                                                              *
*    Media Item Statistics                                                     *
*                                                                              *
********************************************************************************
*/
static MEDIA::StatisticTagMap readID3v2Tags( TagLib::ID3v2::Tag *tag )
{
    MEDIA::StatisticTagMap map;

    TagLib::ID3v2::FrameList list = tag->frameList();
    for( TagLib::ID3v2::FrameList::ConstIterator it = list.begin(); it != list.end(); ++it )
    {
        TagLib::String frameName  = TagLib::String( ( *it )->frameID() );

        if( frameName == POPM_Frame )
        {
            TagLib::ID3v2::PopularimeterFrame *frame =
                    dynamic_cast< TagLib::ID3v2::PopularimeterFrame * >( *it );

            if( !frame ) continue;

            if( TStringToQString( frame->email() ).isEmpty() ) // only read anonymous ratings
            {
                // FMPS tags have precedence
                if( !map.contains( MEDIA::Rating_Tag ) && frame->rating() != 0 )
                    map.insert( MEDIA::Rating_Tag, qRound( frame->rating() / 256.0 * 10.0 ) );
                if( !map.contains( MEDIA::Playcount_Tag ) && frame->counter() < 10000 )
                    map.insert( MEDIA::Playcount_Tag, frame->counter() );
            }
        }
        else if( frameName == TXXX_Frame )
        {
            TagLib::ID3v2::UserTextIdentificationFrame *frame =
                    dynamic_cast< TagLib::ID3v2::UserTextIdentificationFrame * >( *it );

            if( !frame ) continue;

            // the value of the user text frame is stored in the
            // second and following fields.
            TagLib::StringList fields = frame->fieldList();
            if( fields.size() >= 2 )
            {
                QString value = TStringToQString( fields[1] );

                if( fields[0] == TagLib::String("FMPS_Rating") )
                    map.insert( MEDIA::Rating_Tag, qRound( value.toFloat() * 10.0 ) );
                else if( fields[0] == TagLib::String("FMPS_Playcount") )
                    map.insert( MEDIA::Playcount_Tag, value.toFloat() );
            }
        }
    }

    return map;
}

static MEDIA::StatisticTagMap readAPETags( TagLib::APE::Tag *tag )
{
    MEDIA::StatisticTagMap map;

    const TagLib::APE::ItemListMap &items = tag->itemListMap();

    if ( items.contains("FMPS_RATING") )
    {
      const QString value = TStringToQString( items["FMPS_RATING"].values()[0] );
      map.insert( MEDIA::Rating_Tag, value.toFloat()* 10.0);
    }

    if ( items.contains("FMPS_PLAYCOUNT") )
    {
      const QString value = TStringToQString( items["FMPS_PLAYCOUNT"].values()[0] );
      map.insert( MEDIA::Playcount_Tag, value.toFloat());
    }

    return map;
}

static MEDIA::StatisticTagMap readXiphTags( TagLib::Ogg::XiphComment *tag )
{
    MEDIA::StatisticTagMap map;

    const TagLib::Ogg::FieldListMap &tagMap = tag->fieldListMap();

    if ( !tagMap["FMPS_RATING"].isEmpty() )
    {
      const QString value = TStringToQString( tagMap["FMPS_RATING"].front() );
      map.insert( MEDIA::Rating_Tag, value.toFloat()* 10.0);
    }

    if ( !tagMap["FMPS_PLAYCOUNT"].isEmpty() )
    {
      const QString value = TStringToQString( tagMap["FMPS_PLAYCOUNT"].front() );
      map.insert( MEDIA::Playcount_Tag, value.toFloat());
    }

    return map;
}



MEDIA::StatisticTagMap
MEDIA::readStatisticTags( TagLib::FileRef fileref )
{
    MEDIA::StatisticTagMap map;

    if ( TagLib::MPEG::File *file = dynamic_cast<TagLib::MPEG::File *>( fileref.file() ) )
    {
        if ( file->ID3v2Tag() )
            map = readID3v2Tags( file->ID3v2Tag() );
        if ( map.isEmpty() && file->APETag() )
            map = readAPETags( file->APETag() );
    }
    else if ( TagLib::Ogg::Vorbis::File *file = dynamic_cast<TagLib::Ogg::Vorbis::File *>( fileref.file() ) )
    {
        if ( file->tag() )
            map = readXiphTags( file->tag() );
    }
    else if( TagLib::Ogg::FLAC::File *file = dynamic_cast< TagLib::Ogg::FLAC::File * >( fileref.file() ) )
    {
        if ( file->tag() )
            map = readXiphTags( file->tag() );
    }
    else if ( TagLib::FLAC::File *file = dynamic_cast<TagLib::FLAC::File *>( fileref.file() ) )
    {
        if ( file->xiphComment() )
            map = readXiphTags( file->xiphComment() );
        if ( map.isEmpty() && file->ID3v2Tag() )
            map = readID3v2Tags( file->ID3v2Tag() );
    }
    else if( TagLib::Ogg::Speex::File *file = dynamic_cast< TagLib::Ogg::Speex::File * >( fileref.file() ) )
    {
        if( file->tag() )
            map = readXiphTags( file->tag() );
    }

    return map;
}
