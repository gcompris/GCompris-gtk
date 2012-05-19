/* gcompris - sugar_share.vala
 *
 * Copyright (C) 2010, Aleksey Lim
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

private const string _PATH = "/org/gcompris/Administration";
private const string _IFACE = "org.gcompris.Administration";

public delegate string GetProfileCallback ();
public delegate void ReportCallback (string date, int duration, string user,
        string board, int level, int sublevel, int status, string comment);
public delegate void GotProfileCallback (string? data);

/**
 * DBus service to interact between Administration teacher activity and
 * students' GCompris sessions
 */
[DBus (name = "org.gcompris.Administration")]
public class Administration : Object {
    /**
     * Teacher's profile was changed
     *
     * Used by students to know when new teacher's profile should be imported.
     */
    public signal void UpdateProfile ();

    /**
     * @param profile   teacher's profile
     */
    public Administration (GetProfileCallback GetProfile_cb,
            ReportCallback Report_cb) {
        _GetProfile_cb = GetProfile_cb;
        _Report_cb = Report_cb;
    }

    /**
     * Return teacher's profile
     */
    public string GetProfile () {
        debug ("GetProfile called on server");
        return _GetProfile_cb ();
    }

    /**
     * Count student's report
     */
    public void Report (string date, int duration, string user, string board,
            int level, int sublevel, int status, string comment) {
        debug ("Report called on server");
        _Report_cb (date, duration, user, board, level, sublevel, status,
                comment);
    }

    private GetProfileCallback _GetProfile_cb;
    private ReportCallback _Report_cb;
}

/**
 * Highlevel client interface to Administration DBus service
 */
public class AdministrationClient {
    /**
     * Create an instance
     *
     * @param address   DBus connection address
     * @param name      DBus name of the server
     */
    public AdministrationClient (string address, string name,
            GotProfileCallback GotProfile_cb) {
        _GotProfile_cb = GotProfile_cb;

        try {
            _connection = DBus.Bus.open (address);
        } catch (Error error) {
            warning ("Cannot open DBus connection: %s", error.message);
            return;
        }

        _server = _connection.get_object (name, _PATH, _IFACE);
        _server.UpdateProfile.connect (_UpdateProfile_cb);
    }

    /**
     * Start importing profile from Administration server
     */
    public void import_profile () {
        if (_server == null)
            return;

        debug ("GetProfile called on client");

        try {
            _server.GetProfile (_GetProfile_cb);
        } catch (Error error) {
            warning ("Cannot get server profile: %s", error.message);
            _GotProfile_cb (null);
        }
    }

    /**
     * Send report to the server
     */
    public void report (string date, int duration, string user, string board,
            int level, int sublevel, int status, string comment) {
        if (_server == null)
            return;

        debug ("Send report");

        _server.Report (date, duration, user, board, level, sublevel, status,
                comment, _Report_cb);
    }

    private void _Report_cb (Error error) {
        if (error != null) {
            warning ("Cannot send report to server: %s", error.message);
        }
    }

    private void _GetProfile_cb (string profile_data, Error error) {
        if (error != null) {
            warning ("Cannot get server profile: %s", error.message);
            _GotProfile_cb (null);
        } else {
            _GotProfile_cb (profile_data);
        }
    }

    private void _UpdateProfile_cb (dynamic DBus.Object sender) {
        import_profile ();
    }

    private dynamic DBus.Object _server;
    private DBus.Connection _connection;
    private GotProfileCallback _GotProfile_cb;
}

/**
 * Highlevel server object to handle Administration DBus service
 */
public class AdministrationServer {
    /**
     * Create an instance
     *
     * @param address   DBus connection address
     */
    public AdministrationServer (string address,
            GetProfileCallback GetProfile_cb, ReportCallback Report_cb) {
        try {
            _connection = DBus.Bus.open (address);
        } catch (Error error) {
            warning ("Cannot open DBus connection: %s", error.message);
            return;
        }
        _server = new Administration (GetProfile_cb, Report_cb);
        _connection.register_object (_PATH, _server);
    }

    /**
     * Emit remote UpdateProfile signal to force students update their profiles
     */
    public void update_profile () {
        if (_server == null)
            return;

        debug ("Force to update students profiles");

        _server.UpdateProfile ();
    }

    private dynamic Administration _server;
    private DBus.Connection _connection;
}
