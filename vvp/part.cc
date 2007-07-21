/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ident "$Id: part.cc,v 1.12 2006/11/16 01:11:26 steve Exp $"

# include  "compile.h"
# include  "part.h"
# include  <stdlib.h>
# include  <limits.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <iostream>
# include  <assert.h>

vvp_fun_part::vvp_fun_part(unsigned base, unsigned wid)
: base_(base), wid_(wid)
{
      net_ = 0;
}

vvp_fun_part::~vvp_fun_part()
{
}

void vvp_fun_part::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      assert(port.port() == 0);

      if (val_ .eeq( bit ))
	    return;

      val_ = bit;

      if (net_ == 0) {
	    net_ = port.ptr();
	    schedule_generic(this, 0, false);
      }
}

/*
 * Handle the case that the part select node is actually fed by a part
 * select assignment. It's not exactly clear what might make this
 * happen, but is does seem to happen and this should have sell
 * defined behavior.
 */
void vvp_fun_part::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				unsigned base, unsigned wid, unsigned vwid)
{
      assert(bit.size() == wid);

      vvp_vector4_t tmp = val_;
      if (tmp.size() == 0)
	    tmp = vvp_vector4_t(vwid);

      assert(tmp.size() == vwid);
      tmp.set_vec(base, bit);
      recv_vec4(port, tmp);
}

void vvp_fun_part::run_run()
{
      vvp_net_t*ptr = net_;
      net_ = 0;

      vvp_vector4_t res (wid_, BIT4_X);
      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    if (idx + base_ < val_.size())
		  res.set_bit(idx, val_.value(base_+idx));
      }
      vvp_send_vec4(ptr->out, res);
}

vvp_fun_part_pv::vvp_fun_part_pv(unsigned b, unsigned w, unsigned v)
: base_(b), wid_(w), vwid_(v)
{
}

vvp_fun_part_pv::~vvp_fun_part_pv()
{
}

void vvp_fun_part_pv::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      assert(port.port() == 0);

      if (bit.size() != wid_) {
	    cerr << "internal error: part_pv data mismatch. "
		 << "base_=" << base_ << ", wid_=" << wid_
		 << ", vwid_=" << vwid_ << ", bit=" << bit
		 << endl;
      }
      assert(bit.size() == wid_);

      vvp_send_vec4_pv(port.ptr()->out, bit, base_, wid_, vwid_);
}

vvp_fun_part_var::vvp_fun_part_var(unsigned w)
: base_(0), wid_(w)
{
}

vvp_fun_part_var::~vvp_fun_part_var()
{
}

void vvp_fun_part_var::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      unsigned long tmp;
      switch (port.port()) {
	  case 0:
	    source_ = bit;
	    break;
	  case 1:
	    tmp = ULONG_MAX;
	    vector4_to_value(bit, tmp);
	    if (tmp == base_) return;
	    base_ = tmp;
	    break;
	  default:
	    assert(0);
	    break;
      }

      vvp_vector4_t res (wid_);

      for (unsigned idx = 0 ;  idx < wid_ ;  idx += 1) {
	    unsigned adr = base_+idx;
	    if (adr >= source_.size())
		  break;

	    res.set_bit(idx, source_.value(adr));
      }

      if (! ref_.eeq(res)) {
	    ref_ = res;
	    vvp_send_vec4(port.ptr()->out, res);
      }
}


/*
 * Given a node functor, create a network node and link it into the
 * netlist. This form assumes nodes with a single input.
 */
void link_node_1(char*label, char*source, vvp_net_fun_t*fun)
{
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, source);
}

void compile_part_select(char*label, char*source,
			 unsigned base, unsigned wid)
{
      vvp_fun_part*fun = new vvp_fun_part(base, wid);
      link_node_1(label, source, fun);
}

void compile_part_select_pv(char*label, char*source,
			    unsigned base, unsigned wid,
			    unsigned vector_wid)
{
      vvp_fun_part_pv*fun = new vvp_fun_part_pv(base, wid, vector_wid);
      link_node_1(label, source, fun);
}

void compile_part_select_var(char*label, char*source, char*var,
			     unsigned wid)
{
      vvp_fun_part_var*fun = new vvp_fun_part_var(wid);
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;

      define_functor_symbol(label, net);
      free(label);

      input_connect(net, 0, source);
      input_connect(net, 1, var);
}

/*
 * $Log: part.cc,v $
 * Revision 1.12  2006/11/16 01:11:26  steve
 *  Support part writes into  part select nodes.
 *
 * Revision 1.11  2006/05/01 18:44:08  steve
 *  Reduce steps to make logic output.
 *
 * Revision 1.10  2006/04/26 04:39:23  steve
 *  Include  bit value in assertion message.
 *
 * Revision 1.9  2005/09/20 00:51:53  steve
 *  Lazy processing of vvp_fun_part functor.
 *
 * Revision 1.8  2005/07/14 23:34:19  steve
 *  gcc4 compile errors.
 *
 * Revision 1.7  2005/06/26 21:08:11  steve
 *  More verbose debugging of part select width errors.
 *
 * Revision 1.6  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.5  2005/05/09 00:36:58  steve
 *  Force part base out of bounds if index is invalid.
 *
 * Revision 1.4  2005/05/08 23:40:14  steve
 *  Add support for variable part select.
 *
 * Revision 1.3  2005/01/09 20:11:16  steve
 *  Add the .part/pv node and related functionality.
 *
 * Revision 1.2  2004/12/29 23:44:39  steve
 *  Fix missing output propagation of part node.
 *
 * Revision 1.1  2004/12/11 02:31:30  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 */
