// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <ossia/editor/dataspace/dataspace_visitors.hpp>
#include <ossia/editor/dataspace/value_with_unit.hpp>
#include <ossia/network/generic/generic_parameter.hpp>
#include <ossia/network/generic/generic_node.hpp>

namespace ossia
{
namespace net
{
parameter_base::~parameter_base()
{
}

static void getAddressFromNode_rec(const node_base& n, std::string& str)
{
  if (auto p = n.get_parent())
  {
    getAddressFromNode_rec(*p, str);
  }
  else
  {
    // we're at the root
    str += n.get_name();
    str += ':';
    return;
  }

  str += '/';
  str += n.get_name();
}

static void getOSCAddressFromNode_rec(const node_base& n, std::string& str)
{
  if (auto p = n.get_parent())
  {
    getOSCAddressFromNode_rec(*p, str);
  }
  else
  {
    // we're at the root
    return;
  }

  str += '/';
  str += n.get_name();
}

static void
getOSCAddressFromNodeWithDevice_rec(const node_base& n, std::string& str)
{
  if (auto p = n.get_parent())
  {
    getOSCAddressFromNode_rec(*p, str);
  }

  str += '/';
  str += n.get_name();
}

std::string address_string_from_node(const ossia::net::node_base& node)
{
  std::string s;
  s.reserve(80);
  getAddressFromNode_rec(node, s);
  if (s.back() == ':') // case of only device.
    s += '/';
  return s;
}

std::string address_string_from_node(const ossia::net::parameter_base& addr)
{
  return address_string_from_node(addr.get_node());
}

std::string osc_parameter_string(const node_base& n)
{
  if (n.get_parent())
  {
    std::string s;
    s.reserve(80);
    getOSCAddressFromNode_rec(n, s);

    return s;
  }
  else
  {
    return "/";
  }
}

std::string osc_parameter_string_with_device(const node_base& n)
{
  if (n.get_parent())
  {
    std::string s;
    s.reserve(80);
    getOSCAddressFromNodeWithDevice_rec(n, s);

    return s;
  }
  else
  {
    return "/";
  }
}

std::string osc_parameter_string(const parameter_base& addr)
{
  return osc_parameter_string(addr.get_node());
}

std::string osc_parameter_string_with_device(const parameter_base& addr)
{
  return osc_parameter_string_with_device(addr.get_node());
}

std::future<void> parameter_base::pull_value_async()
{
  return {};
}

void parameter_base::request_value()
{
}

value parameter_base::value(destination_index idx) const
{
  return get_value_at_index(value(), idx);
}

std::vector<ossia::value>
parameter_base::value(const std::vector<destination_index>& indices) const
{
  std::vector<ossia::value> t;
  t.reserve(indices.size());

  auto v = value();
  for (auto idx : indices)
  {
    t.push_back(get_value_at_index(v, idx));
  }

  return t;
}

value parameter_base::fetch_value()
{
  pull_value();
  return value();
}


ossia::repetition_filter parameter_base::get_repetition_filter() const
{
  return m_repetitionFilter;
}

ossia::net::parameter_base& parameter_base::set_repetition_filter(
    ossia::repetition_filter repetitionFilter)
{
  if (m_repetitionFilter != repetitionFilter)
  {
    m_repetitionFilter = repetitionFilter;
    m_node.get_device().on_attribute_modified(
        m_node, text_repetition_filter());
  }
  return *this;
}


unit_t parameter_base::get_unit() const
{
  return m_unit;
}

parameter_base& parameter_base::set_unit(const unit_t& u)
{
  if (m_unit != u)
  {
    m_unit = u;
    m_node.get_device().on_attribute_modified(
        m_node, text_unit());
  }
  return *this;
}

bool parameter_base::get_disabled() const
{
  return m_disabled;
}

parameter_base& parameter_base::set_disabled(bool v)
{
  if (m_disabled != v)
  {
    m_disabled = v;
    m_node.get_device().on_attribute_modified(m_node, text_disabled());
  }
  return *this;
}

bool parameter_base::get_muted() const
{
  return m_muted;
}

parameter_base& parameter_base::set_muted(bool v)
{
  if (m_muted != v)
  {
    m_muted = v;
    m_node.get_device().on_attribute_modified(m_node, text_muted());
  }
  return *this;
}

bool parameter_base::get_critical() const
{
  return m_critical;
}

parameter_base& parameter_base::set_critical(bool v)
{
  if (m_critical != v)
  {
    m_critical = v;
    m_node.get_device().on_attribute_modified(m_node, text_critical());
  }
  return *this;
}

value_with_unit get_value(const ossia::destination& d)
{
  ossia::net::parameter_base& addr = d.value.get();

  return make_value(addr.value(d.index), addr.get_unit());
}

void push_value(const destination& d, const value_with_unit& v)
{
  ossia::net::parameter_base& addr = d.value.get();
  addr.push_value(ossia::to_value(v)); // TODO what about destination_index ??
}

std::ostream& operator<<(std::ostream& s, const ossia::net::parameter_base& addr)
{
  return s << ossia::net::address_string_from_node(addr);
}
}
}
