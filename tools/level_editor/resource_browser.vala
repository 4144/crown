/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gtk;

namespace Crown
{
	public class ResourceBrowser : Gtk.Window
	{
		// Project paths
		private string _source_dir;
		private string _bundle_dir;

		// Data
		private GLib.Subprocess _engine_process;
		private ConsoleClient _console_client;

		// Widgets
		private Gtk.Entry _filter_entry;
		private Gtk.TreeStore _tree_store;
		private Gtk.TreeModelFilter _tree_filter;
		private Gtk.TreeView _tree_view;
		private Gtk.ScrolledWindow _scrolled_window;
		private Gtk.Box _box;

		private EngineView _engine_view;

		// Signals
		public signal void resource_selected(PlaceableType placeable_type, string name);

		public ResourceBrowser(string source_dir, string bundle_dir)
		{
			this.title = "Resource Browser";

			// Project paths
			_source_dir = source_dir;
			_bundle_dir = bundle_dir;

			// Data
			_console_client = new ConsoleClient();

			// Widgets
			this.destroy.connect(on_destroy);

			_filter_entry = new Gtk.SearchEntry();
			_filter_entry.set_placeholder_text("Search...");
			_filter_entry.changed.connect(on_filter_entry_text_changed);

			_tree_store = new Gtk.TreeStore(2, typeof(string), typeof(PlaceableType));
			_tree_filter = new Gtk.TreeModelFilter(_tree_store, null);
			_tree_filter.set_visible_func(filter_tree);

			_tree_view = new Gtk.TreeView();
			_tree_view.insert_column_with_attributes(-1, "Name", new Gtk.CellRendererText(), "text", 0, null);
			_tree_view.model = _tree_filter;
			_tree_view.headers_visible = false;
			_tree_view.row_activated.connect(on_row_activated);
			_tree_view.button_press_event.connect(on_button_pressed);

			_scrolled_window = new Gtk.ScrolledWindow(null, null);
			_scrolled_window.add(_tree_view);
			_scrolled_window.set_size_request(300, 400);

			walk_directory_tree();

			_engine_view = new EngineView(_console_client, false);
			_engine_view.realized.connect(on_engine_view_realized);

			_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
			_box.pack_start(_filter_entry, false, false, 0);
			_box.pack_start(_scrolled_window, false, false, 0);
			_box.pack_start(_engine_view, true, true, 0);

			resource_selected.connect(on_resource_selected);

			add(_box);
			set_size_request(300, 400);
		}

		private void on_row_activated(Gtk.TreePath path, TreeViewColumn column)
		{
			Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(path);
			Gtk.TreeIter iter;
			if (_tree_store.get_iter(out iter, child_path))
			{
				Value name;
				Value type;
				_tree_store.get_value(iter, 0, out name);
				_tree_store.get_value(iter, 1, out type);
				resource_selected((PlaceableType)type, (string)name);
			}
		}

		private bool on_button_pressed(Gdk.EventButton ev)
		{
			int button = (int)ev.button;

			if (button == 1)
			{
				Gtk.TreePath path;
				int cell_x;
				int cell_y;
				if (_tree_view.get_path_at_pos((int)ev.x, (int)ev.y, out path, null, out cell_x, out cell_y))
				{
					Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(path);
					Gtk.TreeIter iter;
					if (_tree_store.get_iter(out iter, child_path))
					{
						Value name;
						Value type;
						_tree_store.get_value(iter, 0, out name);
						_tree_store.get_value(iter, 1, out type);
						resource_selected((PlaceableType)type, (string)name);
					}
				}
			}

			return false;
		}

		private void on_destroy()
		{
			if (_console_client.is_connected())
				_console_client.close();

			if (_engine_process != null)
				_engine_process.force_exit();
		}

		private void start_engine(uint window_xid)
		{
			string args[] =
			{
				ENGINE_EXE,
				"--bundle-dir", _bundle_dir,
				"--boot-dir", "core/unit_preview",
				"--parent-window", window_xid.to_string(),
				"--console-port", "10002",
				"--wait-console",
				null
			};

			GLib.SubprocessLauncher sl = new GLib.SubprocessLauncher(SubprocessFlags.NONE);
			sl.set_cwd(ENGINE_DIR);
			try
			{
				_engine_process = sl.spawnv(args);
			}
			catch (Error e)
			{
				GLib.stderr.printf("%s\n", e.message);
			}

			while (!_console_client.is_connected())
				_console_client.connect("127.0.0.1", 10002);
		}

		private void on_engine_view_realized()
		{
			start_engine(_engine_view.window_id);
		}

		private void on_filter_entry_text_changed()
		{
			_tree_filter.refilter();

			Gtk.TreePath path = new Gtk.TreePath.from_string("0");
			Gtk.TreePath child_path = _tree_filter.convert_path_to_child_path(path);
			Gtk.TreeIter iter;
			if (_tree_store.get_iter(out iter, child_path))
			{
				Value name;
				Value type;
				_tree_store.get_value(iter, 0, out name);
				_tree_store.get_value(iter, 1, out type);
				resource_selected((PlaceableType)type, (string)name);
			}
		}

		private bool filter_tree(Gtk.TreeModel model, Gtk.TreeIter iter)
		{
			Value name;
			model.get_value(iter, 0, out name);

			if (_filter_entry.text == "")
					 return true;

			string text = (string)name;
			if (text.index_of(_filter_entry.text) > -1)
				return true;
			else
				return false;
		}

		private void walk_directory_tree()
		{
			_tree_view.model = null;
			File file = File.new_for_path(_source_dir);
			list_children(file);
			_tree_view.model = _tree_filter;
		}

		private void list_children(File file, string space = "", Cancellable? cancellable = null) throws Error
		{
			FileEnumerator enumerator = file.enumerate_children ("standard::*", FileQueryInfoFlags.NOFOLLOW_SYMLINKS, cancellable);

			FileInfo info = null;
			while (cancellable.is_cancelled () == false && ((info = enumerator.next_file (cancellable)) != null)) {
				if (info.get_file_type () == FileType.DIRECTORY)
				{
					File subdir = file.resolve_relative_path (info.get_name ());
					list_children (subdir, space + " ", cancellable);
				}
				else
				{
					string path = file.get_path() + "/" + info.get_name();
					string path_rel = File.new_for_path(_source_dir).get_relative_path(File.new_for_path(path));
					string type = path_rel.substring(path_rel.last_index_of("."), path_rel.length - path_rel.last_index_of("."));
					string name = path_rel.substring(0, path_rel.last_index_of("."));

					if (type == ".unit" || type == ".sound")
					{
						Gtk.TreeIter resource_iter;
						_tree_store.append(out resource_iter, null);
						_tree_store.set(resource_iter, 0, name, 1, type == ".unit" ? PlaceableType.UNIT : PlaceableType.SOUND, -1);
					}
				}
			}

			if (cancellable.is_cancelled ())
			{
				throw new IOError.CANCELLED("Operation was cancelled");
			}
		}

		private void on_resource_selected(PlaceableType placeable_type, string name)
		{
			_console_client.send_script(UnitPreviewAPI.set_preview_unit(placeable_type, name));
		}
	}
}
