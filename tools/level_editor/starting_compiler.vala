/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
	public class StartingCompiler : Gtk.Bin
	{
		public StartingCompiler()
		{
			add(new Gtk.Label("Compiling resources, please wait..."));
			set_size_request(300, 300);
			show_all();
		}
	}
}
