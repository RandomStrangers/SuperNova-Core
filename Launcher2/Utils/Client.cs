﻿// ClassicalSharp copyright 2014-2016 UnknownShadow200 | Licensed under MIT
using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using ClassicalSharp;
using OpenTK;

namespace Launcher {

	public static class Client {
		static DateTime lastJoin;
		public static bool CClient;
		
		public static string GetExeName() {
			if (!CClient) return "ClassicalSharp.exe";
			
			if (OpenTK.Configuration.RunningOnWindows)
				return "ClassiCube.exe";
			return "ClassiCube"; // TODO: OSX filename
		}
		
		public static bool Start(ClientStartData data, bool classicubeSkins, ref bool shouldExit) {
			if ((DateTime.UtcNow - lastJoin).TotalSeconds < 1)
				return false;
			lastJoin = DateTime.UtcNow;
			
			string skinServer = classicubeSkins ? "http://cdn.classicube.net/skin/" :
				"https://minotar.net/skin/";
			string args = data.Username + " " + data.Mppass + " " +
				data.Ip + " " + data.Port + " " + skinServer;
			return StartImpl(data, classicubeSkins, args, ref shouldExit);
		}
		
		public static bool Start(string args, ref bool shouldExit) {
			return StartImpl(null, true, args, ref shouldExit);
		}
		
		static bool StartImpl(ClientStartData data, bool ccSkins, string args, ref bool shouldExit) {
			if (!Platform.FileExists(GetExeName())) return false;
			
			UpdateSettings(data, ccSkins, out shouldExit);
			try {
				StartProcess(args);
			} catch (Win32Exception ex) {
				if ((uint)ex.ErrorCode != 0x80004005)
					throw; // HRESULT when user clicks 'cancel' to 'are you sure you want to run ClassicalSharp.exe'
				shouldExit = false;
				return false;
			}
			return true;
		}
		
		static void StartProcess(string args) {
			string path = Path.Combine(Environment.CurrentDirectory, GetExeName());
			
			if (Configuration.RunningOnMono && !CClient) {
				// We also need to handle the case of running Mono through wine
				if (Configuration.RunningOnWindows) {
					try {
						Process.Start("mono", "\"" + path + "\" " + args);
					} catch (Win32Exception ex) {
						if (!((uint)ex.ErrorCode == 0x80070002 || (uint)ex.ErrorCode == 0x80004005))
							throw; // File not found HRESULT, HRESULT thrown when running on wine
						Process.Start(path, args);
					}
				} else if (Configuration.RunningOnMacOS && NeedOSXHack()) {
					Process.Start("mono", "--arch=32 \"" + path + "\" " + args);
				} else {
					Process.Start("mono", "\"" + path + "\" " + args);
				}
			} else {
				Process.Start(path, args);
			}
		}
		
		static bool NeedOSXHack() {
			Type type = Type.GetType("Mono.Runtime");
			if (type == null) return false;
			MethodInfo displayName = type.GetMethod("GetDisplayName", BindingFlags.NonPublic | BindingFlags.Static);
			if (displayName == null) return false;
			
			object versionRaw = displayName.Invoke(null, null);
			if (versionRaw == null) return false;
			
			string versionStr = versionRaw.ToString();
			if (versionStr.IndexOf(' ') >= 0) {
				versionStr = versionStr.Substring(0, versionStr.IndexOf(' '));
			}
			
			try {
				Version version = new Version(versionStr);
				return version.Major >= 5 && version.Minor >= 2;
			} catch {
				return false;
			}
		}
		
		static void UpdateSettings(ClientStartData data, bool ccSkins, out bool shouldExit) {
			shouldExit = false;
			// Make sure if the client has changed some settings in the meantime, we keep the changes
			if (!Options.Load())
				return;
			shouldExit = Options.GetBool(OptionsKey.AutoCloseLauncher, false);
			if (data == null) return;
			
			Options.Set("launcher-server", data.Server);
			Options.Set("launcher-username", data.Username);
			Options.Set("launcher-ip", data.Ip);
			Options.Set("launcher-port", data.Port);
			Options.Set("launcher-mppass", Secure.Encode(data.Mppass, data.Username));
			Options.Set("launcher-ccskins", ccSkins);
			Options.Save();
		}
	}

	public class ClientStartData {
		public string Username, Mppass, Ip, Port, Server;
		
		public ClientStartData(string user, string mppass, string ip, string port, string server) {
			Username = user;
			Mppass = mppass;
			Ip = ip;
			Port = port;
			Server = server;
		}
	}
}
