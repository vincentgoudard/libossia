// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "view.hpp"
#include "remote.hpp"
#include "utils.hpp"

using namespace ossia::max;

#pragma mark -
#pragma mark ossia_remote class methods

extern "C" void ossia_view_setup()
{
  // instantiate the ossia.view class
  t_class* c = class_new(
      "ossia.view", (method)view::create, (method)view::destroy,
      (short)sizeof(view), 0L, A_GIMME, 0);

  if (c)
  {
    class_addmethod(c, (method) address_mess_cb<view>, "address",   A_SYM, 0);
    class_addmethod(c, (method) view::get_mess_cb, "get",   A_SYM, 0);

    class_addmethod(
          c, (method)view::notify,
          "notify", A_CANT, 0);
  }

  class_register(CLASS_BOX, c);

  auto& ossia_library = ossia_max::instance();
  ossia_library.ossia_view_class = c;
}

namespace ossia
{
namespace max
{

void* view::create(t_symbol* name, long argc, t_atom* argv)
{
  auto x = make_ossia<view>();

  if (x)
  {
    auto& pat_desc = ossia_max::instance().patchers[x->m_patcher];
    if( !pat_desc.model && !pat_desc.view)
    {
      pat_desc.view = x;
    }
    else
    {
      error("You can put only one [ossia.model] or [ossia.view] per patcher");
      object_free(x);
      return nullptr;
    }

    x->m_otype = object_class::view;

    // make outlets
    x->m_dumpout = outlet_new(x, NULL); // anything outlet to dump view state

    // parse arguments
    long attrstart = attr_args_offset(argc, argv);

    if(find_peer(x))
    {
      error("You can put only one [ossia.model] or [ossia.view] per patcher");
      view::destroy(x);
      return nullptr;
    }

    // check name argument
    x->m_name = _sym_nothing;
    if (attrstart && argv)
    {
      if (atom_gettype(argv) == A_SYM)
      {
        x->m_name = atom_getsym(argv);
        x->m_addr_scope = ossia::net::get_address_scope(x->m_name->s_name);
      }
    }

    // process attr args, if any
    attr_args_process(x, argc - attrstart, argv + attrstart);

    // need to schedule a loadbang because objects only receive a loadbang when patcher loads.
    x->m_reg_clock = clock_new(x, (method) object_base::loadbang);
    clock_set(x->m_reg_clock, 1);
  }

  ossia_max::instance().views.push_back(x);

  return (x);
}

void view::destroy(view* x)
{
  auto pat_it = ossia_max::instance().patchers.find(x->m_patcher);
  if(pat_it != ossia_max::instance().patchers.end())
  {
    auto& pat_desc = pat_it->second;
    if(pat_desc.view  == x)
      pat_desc.view = nullptr;
    if(pat_desc.empty())
    {
      ossia_max::instance().patchers.erase(pat_it);
    }
  }

  x->m_dead = true;
  x->unregister();
  ossia_max::instance().views.remove_all(x);
  if(x->m_clock)
    object_free(x->m_clock);
  if(x->m_dumpout)
    outlet_delete(x->m_dumpout);
  x->~view();
}

void view::do_registration(const std::vector<std::shared_ptr<matcher>>& matchers)
{
  // FIXME review absolute and global scope registering

  if(!m_registered)
  {
    m_registered = true;

    switch(m_addr_scope)
    {
      case ossia::net::address_scope::absolute:
      case ossia::net::address_scope::global:
        object_error(&m_object, "remote with glboal/absolute path are not supported yet");
        return;
      default:
          ;
    }
  }

  m_matchers.clear();
  m_matchers.reserve(matchers.size());

  for (auto& m : matchers)
  {
    auto _node = m->get_node();
    std::string name = m_name->s_name;

    if (m_addr_scope == ossia::net::address_scope::absolute)
    {
      // get root node
      _node = &_node->get_device().get_root_node();
      // and remove starting '/'
      name = name.substr(1);
    }

    std::vector<ossia::net::node_base*> nodes{};

    if (m_addr_scope == ossia::net::address_scope::global)
      nodes = ossia::max::find_global_nodes(name);
    else
      nodes = ossia::net::find_nodes(*_node, name);

    m_matchers.reserve(m_matchers.size() + nodes.size());

    for (auto n : nodes)
    {
      // we may have found a node with the same name
      // but with a parameter, in that case it's an ø.param
      // then forget it
      if (!n->get_parameter())
        m_matchers.emplace_back(std::make_shared<matcher>(n, this));
    }
  }

  fill_selection();
}

void view::unregister()
{
  m_matchers.clear();

  std::vector<object_base*> children_view = find_children_to_register(
      &m_object, m_patcher, gensym("ossia.view"));

  for (auto child : children_view)
  {
    if (child->m_otype == object_class::view)
    {
      ossia::max::view* view = (ossia::max::view*)child;

      if (view == this)
        continue;

      view->unregister();
    }
    else if (child->m_otype == object_class::remote)
    {
      ossia::max::remote* remote = (ossia::max::remote*)child;
      remote->unregister();
    }
  }

  ossia_max::instance().nr_views.push_back(this);

  // register_children(this);

  m_registered = false;
}

} // max namespace
} // ossia namespace
