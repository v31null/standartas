using System;
using System.IO;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace psnpn1117r2_test
{
    unsafe static class psnpn1117r2_quic
    {
        [DllImport("msquic.dll")] static extern int MsQuicOpenVersion(uint version, out IntPtr api);
        [DllImport("msquic.dll")] static extern void MsQuicClose(IntPtr api);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int Handler(IntPtr handle, IntPtr context, IntPtr evt);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int SetParamFn(IntPtr h, uint param, uint len, void* buf);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int RegistrationOpenFn(void* config, out IntPtr reg);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void CloseFn(IntPtr h);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int ConfigurationOpenFn(IntPtr reg, void* alpns, uint alpnCount, void* settings, uint settingsSize, IntPtr ctx, out IntPtr config);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int LoadCredentialFn(IntPtr config, void* cred);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int ConnectionOpenFn(IntPtr reg, IntPtr handler, IntPtr ctx, out IntPtr c);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate void ConnectionShutdownFn(IntPtr c, uint flags, ulong code);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int ConnectionStartFn(IntPtr c, IntPtr config, ushort family, [MarshalAs(UnmanagedType.LPStr)] string server, ushort port);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int StreamOpenFn(IntPtr c, uint flags, IntPtr handler, IntPtr ctx, out IntPtr s);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int StreamStartFn(IntPtr s, uint flags);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)] delegate int StreamSendFn(IntPtr s, void* bufs, uint count, uint flags, IntPtr ctx);

        const uint QUIC_PARAM_CONN_LOCAL_ADDRESS = 0x05000001;
        const uint QUIC_PARAM_CONN_SHARE_UDP_BINDING = 0x05000007;
        const int FLAG_CLIENT = 0x1, FLAG_NO_VALIDATION = 0x4;
        const ushort AF_INET = 2;
        const int CONNECTED = 0, CONNECTION_SHUTDOWN_COMPLETE = 3;
        const int RECEIVE = 1, SEND_COMPLETE = 2, STREAM_SHUTDOWN_COMPLETE = 7;

        static readonly Handler connectionHandler = OnConnection;
        static readonly Handler streamHandler = OnStream;
        static readonly ManualResetEvent connected = new ManualResetEvent(false);
        static readonly ManualResetEvent answered = new ManualResetEvent(false);
        static readonly ManualResetEvent closed = new ManualResetEvent(false);
        static readonly MemoryStream received = new MemoryStream();
        static CloseFn connectionClose, streamClose;
        static int wanted;
        static byte[] answer;

        static T Fn<T>(IntPtr api, int index)
        {
            return Marshal.GetDelegateForFunctionPointer<T>(Marshal.ReadIntPtr(api, index * IntPtr.Size));
        }

        static void Check(int status)
        {
            if (status < 0) throw new Exception("0x" + status.ToString("x8"));
        }

        public static byte[] Ask(IPEndPoint to, int localPort, int type, byte[] body)
        {
            IntPtr api;
            Check(MsQuicOpenVersion(2, out api));
            var setParam = Fn<SetParamFn>(api, 3);
            var registrationOpen = Fn<RegistrationOpenFn>(api, 5);
            var registrationClose = Fn<CloseFn>(api, 6);
            var configurationOpen = Fn<ConfigurationOpenFn>(api, 8);
            var configurationClose = Fn<CloseFn>(api, 9);
            var loadCredential = Fn<LoadCredentialFn>(api, 10);
            var connectionOpen = Fn<ConnectionOpenFn>(api, 15);
            connectionClose = Fn<CloseFn>(api, 16);
            var connectionShutdown = Fn<ConnectionShutdownFn>(api, 17);
            var connectionStart = Fn<ConnectionStartFn>(api, 18);
            var streamOpen = Fn<StreamOpenFn>(api, 21);
            streamClose = Fn<CloseFn>(api, 22);
            var streamStart = Fn<StreamStartFn>(api, 23);
            var streamSend = Fn<StreamSendFn>(api, 25);

            var regConfig = stackalloc IntPtr[2];
            regConfig[0] = Marshal.StringToHGlobalAnsi("psnpn1117r2_test");
            regConfig[1] = IntPtr.Zero;
            IntPtr registration;
            Check(registrationOpen(regConfig, out registration));

            var alpn = Encoding.ASCII.GetBytes("pronal1");
            var alpnData = Marshal.AllocHGlobal(alpn.Length);
            Marshal.Copy(alpn, 0, alpnData, alpn.Length);
            var alpnBuffer = stackalloc byte[16];
            *(uint*)alpnBuffer = (uint)alpn.Length;
            *(IntPtr*)(alpnBuffer + 8) = alpnData;
            IntPtr config;
            Check(configurationOpen(registration, alpnBuffer, 1, null, 0, IntPtr.Zero, out config));

            var cred = stackalloc byte[56];
            for (var i = 0; i < 56; i++) cred[i] = 0;
            *(int*)(cred + 4) = FLAG_CLIENT | FLAG_NO_VALIDATION;
            Check(loadCredential(config, cred));

            IntPtr conn;
            Check(connectionOpen(registration, Marshal.GetFunctionPointerForDelegate(connectionHandler), IntPtr.Zero, out conn));
            wanted = type;
            try
            {
                byte share = 1;
                Check(setParam(conn, QUIC_PARAM_CONN_SHARE_UDP_BINDING, 1, &share));
                var local = stackalloc byte[28];
                for (var i = 0; i < 28; i++) local[i] = 0;
                local[0] = (byte)AF_INET;
                local[2] = (byte)(localPort >> 8);
                local[3] = (byte)localPort;
                Check(setParam(conn, QUIC_PARAM_CONN_LOCAL_ADDRESS, 28, local));
                Check(connectionStart(conn, config, AF_INET, to.Address.ToString(), (ushort)to.Port));
                if (!connected.WaitOne(10000)) throw new Exception();

                IntPtr stream;
                Check(streamOpen(conn, 0, Marshal.GetFunctionPointerForDelegate(streamHandler), conn, out stream));
                Check(streamStart(stream, 0));
                var frame = new byte[5 + body.Length];
                frame[0] = (byte)type;
                frame[1] = (byte)(body.Length >> 24);
                frame[2] = (byte)(body.Length >> 16);
                frame[3] = (byte)(body.Length >> 8);
                frame[4] = (byte)body.Length;
                Buffer.BlockCopy(body, 0, frame, 5, body.Length);
                var mem = Marshal.AllocHGlobal(16 + frame.Length);
                Marshal.WriteInt32(mem, 0, frame.Length);
                Marshal.WriteIntPtr(mem, 8, mem + 16);
                Marshal.Copy(frame, 0, mem + 16, frame.Length);
                Check(streamSend(stream, (void*)mem, 1, 0, mem));
                if (!answered.WaitOne(10000)) throw new Exception();
                return answer;
            }
            finally
            {
                if (!closed.WaitOne(0)) connectionShutdown(conn, 0, 0);
                if (closed.WaitOne(3000))
                {
                    configurationClose(config);
                    registrationClose(registration);
                    MsQuicClose(api);
                }
            }
        }

        static int OnConnection(IntPtr conn, IntPtr ctx, IntPtr evt)
        {
            switch (Marshal.ReadInt32(evt))
            {
                case CONNECTED:
                    connected.Set();
                    break;
                case CONNECTION_SHUTDOWN_COMPLETE:
                    connectionClose(conn);
                    closed.Set();
                    break;
            }
            return 0;
        }

        static int OnStream(IntPtr stream, IntPtr conn, IntPtr evt)
        {
            switch (Marshal.ReadInt32(evt))
            {
                case RECEIVE:
                    var count = Marshal.ReadInt32(evt, 32);
                    var bufs = Marshal.ReadIntPtr(evt, 24);
                    for (var i = 0; i < count; i++)
                    {
                        var b = bufs + i * 16;
                        var data = new byte[Marshal.ReadInt32(b)];
                        Marshal.Copy(Marshal.ReadIntPtr(b, 8), data, 0, data.Length);
                        received.Write(data, 0, data.Length);
                    }
                    var all = received.ToArray();
                    if (all.Length >= 5)
                    {
                        var size = (all[1] << 24) | (all[2] << 16) | (all[3] << 8) | all[4];
                        if (all[0] == wanted && size >= 0 && all.Length >= 5 + size)
                        {
                            answer = new byte[size];
                            Buffer.BlockCopy(all, 5, answer, 0, size);
                            answered.Set();
                        }
                    }
                    break;
                case SEND_COMPLETE:
                    Marshal.FreeHGlobal(Marshal.ReadIntPtr(evt, 16));
                    break;
                case STREAM_SHUTDOWN_COMPLETE:
                    streamClose(stream);
                    break;
            }
            return 0;
        }
    }
}
