using System;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;

namespace psnpn1117r2_test
{
    static class psnpn1117r2_Ż_ù__Links_Across_the_Internet
    {
        public static IPEndPoint Ask(UdpClient udp, string hostPort)
        {
            var colon = hostPort.LastIndexOf(':');
            var server = new IPEndPoint(Dns.GetHostAddresses(hostPort.Substring(0, colon)).First(a => a.AddressFamily == AddressFamily.InterNetwork), int.Parse(hostPort.Substring(colon + 1)));
            var tx = new byte[12];
            using (var rng = RandomNumberGenerator.Create()) rng.GetBytes(tx);
            var req = new byte[20];
            req[1] = 1;
            req[4] = 0x21; req[5] = 0x12; req[6] = 0xA4; req[7] = 0x42;
            Buffer.BlockCopy(tx, 0, req, 8, 12);
            for (var attempt = 0; attempt < 3; attempt++)
            {
                udp.Send(req, req.Length, server);
                try
                {
                    var from = new IPEndPoint(IPAddress.Any, 0);
                    var res = udp.Receive(ref from);
                    if (res.Length < 20 || !res.Skip(8).Take(12).SequenceEqual(tx)) continue;
                    var pos = 20;
                    while (pos + 4 <= res.Length)
                    {
                        var type = (res[pos] << 8) | res[pos + 1];
                        var len = (res[pos + 2] << 8) | res[pos + 3];
                        if ((type == 0x0020 || type == 0x0001) && pos + 12 <= res.Length && res[pos + 5] == 1)
                        {
                            var port = (res[pos + 6] << 8) | res[pos + 7];
                            var ip = res.Skip(pos + 8).Take(4).ToArray();
                            if (type == 0x0020)
                            {
                                port ^= 0x2112;
                                ip[0] ^= 0x21; ip[1] ^= 0x12; ip[2] ^= 0xA4; ip[3] ^= 0x42;
                            }
                            return new IPEndPoint(new IPAddress(ip), port);
                        }
                        pos += 4 + ((len + 3) & ~3);
                    }
                }
                catch (SocketException) { }
            }
            return null;
        }
    }
}
