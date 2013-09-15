/*
 Copyright (c) 2012 The VCT Project

  This file is part of VoxelConeTracing and is an implementation of
  "Interactive Indirect Illumination Using Voxel Cone Tracing" by Crassin et al

  VoxelConeTracing is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  VoxelConeTracing is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with VoxelConeTracing.  If not, see <http://www.gnu.org/licenses/>.
*/

/*!
* \author Dominik Lazarek (dominik.lazarek@gmail.com)
* \author Andreas Weinmann (andy.weinmann@gmail.com)
*/


#include "KoRE/GPUtimer.h"

using namespace kore;

#define TIMING_NOT_AVAILABLE 0xFFFFFFFFFFFFFFFF

GPUtimer* GPUtimer::getInstance() {
  static GPUtimer instance;
  return &instance;
}


GPUtimer::GPUtimer() {

}

GPUtimer::~GPUtimer() {
  
  for (auto iter = _queryObjects.begin(); iter != _queryObjects.end(); iter++) {
    GLuint val = (*iter);
    glDeleteQueries(1, &val);
  }
}

void GPUtimer::queryTimestamp(const std::string& name, GLuint queryObject) {
  if (std::find(_queryObjects.begin(), _queryObjects.end(), queryObject) 
    == _queryObjects.end())
  {
      _queryObjects.push_back(queryObject);
      _queryNames[queryObject] = name;
      glQueryCounter(queryObject, GL_TIMESTAMP);
  }
}

void GPUtimer::checkQueryResults() {
  std::vector<GLuint> finishedList;

  for (auto iter = _queryObjects.begin(); iter != _queryObjects.end(); iter++) {

    GLuint available = GL_FALSE;
    glGetQueryObjectuiv((*iter), GL_QUERY_RESULT_AVAILABLE, &available);
    
    if (available) {
      finishedList.push_back((*iter));
    }
  }

  for (uint i = 0; i < finishedList.size(); ++i) {
    // Remove from queryObjects-list
    _queryObjects.erase(std::find(_queryObjects.begin(),
                                  _queryObjects.end(),
                                  finishedList[i]));


    GLuint64 result;
    glGetQueryObjectui64v(finishedList[i], GL_QUERY_RESULT, &result);
    _timestamps[finishedList[i]] = result;
  }
}


GLuint64 GPUtimer::getQueryResult(const uint queryID) {
  if (isTimestampAvailable(queryID)) {
    return _timestamps[queryID];
  }

  return TIMING_NOT_AVAILABLE;
}

const std::string& GPUtimer::getQueryName(const uint queryID) {
  return _queryNames.find(queryID)->second;
}

void GPUtimer::removeQueryResult(const uint queryID) {
   
   glDeleteQueries(1, &queryID);
   
   _queryNames.erase(_queryNames.find(queryID));
   _timestamps.erase(_timestamps.find(queryID));
   _durationQueries.erase(_durationQueries.find(queryID));
}


void GPUtimer::startDurationQuery(const std::string& name, GLuint queryObject) {
  queryTimestamp(name, queryObject);
}

void GPUtimer::endDurationQuery(const uint startQueryID) {
  auto iter = _durationQueries.find(startQueryID);

  GLuint endQuery = 0;
  if (iter == _durationQueries.end()) {
    // No entry in the durationQuery-table yet. 
    glGenQueries(1, &endQuery);
  } else {
     endQuery = iter->second;
  }

  _durationQueries[startQueryID] = endQuery;
  queryTimestamp("", endQuery);
}

GLuint64 GPUtimer::getDurationMS(const uint startQueryID) {
  auto iter = _durationQueries.find(startQueryID);

  if (iter == _durationQueries.end()) {
    // No entry in the durationQuery-table. 
    // endDurationQUery() wasn't called yet
    return TIMING_NOT_AVAILABLE;
  }

  GLuint endQueryID = iter->second;

  if (isTimestampAvailable(startQueryID) 
      && isTimestampAvailable(endQueryID)) {
        GLuint64 duration = 
          static_cast<double>(getQueryResult(endQueryID) - getQueryResult(startQueryID));

        return duration;
  }

  return TIMING_NOT_AVAILABLE;  // Timestamps are not both available (yet)
}

void GPUtimer::getDurationResultsMS(std::vector<SDurationResult>& rvResults) {
  rvResults.clear();
  for (auto iter = _durationQueries.begin(); iter != _durationQueries.end(); iter++) {
    GLuint queryID = iter->first;
    GLuint64 duration = getDurationMS(queryID);

    if (duration != TIMING_NOT_AVAILABLE) {
      SDurationResult currResult;
      currResult.durationNS = duration;
      currResult.name = getQueryName(queryID);
      currResult.startQueryID = queryID;
      rvResults.push_back(currResult);
    }
  }

}

void GPUtimer::removeDurationQuery(const uint startQueryID) {
  auto iter = _durationQueries.find(startQueryID);

  if (iter == _durationQueries.end()) {
    // No entry in the durationQuery-table. 
    // This queryID was not created using startDurationQuery...
    return;
  }

  uint endQueryID = iter->second;
  removeQueryResult(startQueryID);
  removeQueryResult(endQueryID);
  _durationQueries.erase(iter);
}


bool kore::GPUtimer::isTimestampAvailable( const uint queryID )
{
  return _timestamps.find(queryID) != _timestamps.end();
}
