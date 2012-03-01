/**
 * @file cxx/io/src/HDF5File.cc
 * @date Wed Jun 22 17:50:08 2011 +0200
 * @author Andre Anjos <andre.anjos@idiap.ch>
 *
 * @brief Implementation of the read/write functionality for HDF5 files
 *
 * Copyright (C) 2011-2012 Idiap Research Institute, Martigny, Switzerland
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "io/HDF5File.h"

namespace io = bob::io;
namespace ca = bob::core::array;

static unsigned int getH5Access (io::HDF5File::mode_t v) {
  switch(v)
  {
    case 0: return H5F_ACC_RDONLY;
    case 1: return H5F_ACC_RDWR;
    case 2: return H5F_ACC_TRUNC;
    case 4: return H5F_ACC_EXCL;
    default:
      throw io::HDF5InvalidFileAccessModeError(v);
  }
}

io::HDF5File::HDF5File(const std::string& filename, mode_t mode):
  m_file(new io::detail::hdf5::File(filename, getH5Access(mode))),
  m_cwd(m_file->root()) ///< we start by looking at the root directory
{
}

io::HDF5File::~HDF5File() {
}

void io::HDF5File::cd(const std::string& path) {
  m_cwd = m_cwd->cd(path);
}

bool io::HDF5File::hasGroup(const std::string& path) {
  return m_cwd->has_group(path);
}

void io::HDF5File::createGroup(const std::string& path) {
  m_cwd->create_group(path);
}

std::string io::HDF5File::cwd() const {
  return m_cwd->path();
}

bool io::HDF5File::contains (const std::string& path) const {
  return m_cwd->has_dataset(path);
}

const std::vector<io::HDF5Descriptor>& io::HDF5File::describe 
(const std::string& path) const {
  return (*m_cwd)[path]->m_descr;
}

void io::HDF5File::unlink (const std::string& path) {
  m_cwd->remove_dataset(path);
}

void io::HDF5File::rename (const std::string& from, const std::string& to) {
  m_cwd->rename_dataset(from, to);
  std::string current_path = m_cwd->path();
  m_file->reset(); //re-read the whole structure
  m_cwd = m_file->root();
  m_cwd = m_cwd->cd(current_path); //go back to the path we were before
}

void io::HDF5File::copy (HDF5File& other) {
  //groups
  typedef std::map<std::string, boost::shared_ptr<io::detail::hdf5::Group> > group_map_type;
  const group_map_type& group_map = other.m_file->root()->groups();
  for (group_map_type::const_iterator it=group_map.begin();
      it != group_map.end(); ++it) {
    m_cwd->copy_group(it->second, it->first);
  }
  
  //datasets
  typedef std::map<std::string, boost::shared_ptr<io::detail::hdf5::Dataset> > dataset_map_type;
  const dataset_map_type& dataset_map = other.m_file->root()->datasets();
  for (dataset_map_type::const_iterator it=dataset_map.begin();
      it != dataset_map.end(); ++it) {
    m_cwd->copy_dataset(it->second, it->first);
  }
}

void io::HDF5File::create (const std::string& path, const ca::typeinfo& ti,
    bool list, size_t compression) {
  if (!contains(path)) m_cwd->create_dataset(path, ti, list, compression);
  else (*m_cwd)[path]->size(io::HDF5Type(ti));
}

void io::HDF5File::read_buffer (const std::string& path, size_t pos, 
    ca::interface& b) {
  (*m_cwd)[path]->read_buffer(pos, io::HDF5Type(b.type()), b.ptr());
}

void io::HDF5File::write_buffer (const std::string& path, 
    size_t pos, const ca::interface& b) {
  (*m_cwd)[path]->write_buffer(pos, io::HDF5Type(b.type()), b.ptr());
}

void io::HDF5File::extend_buffer(const std::string& path,
    const ca::interface& b) {
  (*m_cwd)[path]->extend_buffer(io::HDF5Type(b.type()), b.ptr());
}
