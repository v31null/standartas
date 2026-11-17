using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Text;
using System.Web.Script.Serialization;

namespace psnpn1117r2_test
{
    static class psnpn1117r2_test
    {
        const string psnpn1117r2_API1 = "https://pronal.v31null.de/API1/";
        const string psnpn1117r2_server_key = "BN/VuTWK/9ELVBRLsUD0OJ5SodGEveLeoo17DgKqqg+FnxGH6HD4pdErsczR6GVAFvD3XLSRbO0oV2AG/ARnpRY=";
        const int psnpn1117r2_Table_Ż_1__reach = 15;

        static readonly JavaScriptSerializer Json = new JavaScriptSerializer();

        static int Main()
        {
            var step = "stun";
            try
            {
                ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;

                var stun = ((object[])Get("mesh/reach.php")["stun"]).Cast<string>().ToList();
                if (stun.Count < 2) throw new Exception();
                Line("stun", string.Join("  ", stun));

                step = "public";
                int dialPort;
                IPEndPoint mine;
                using (var udp = new UdpClient(new IPEndPoint(IPAddress.Any, 0)))
                {
                    udp.Client.ReceiveTimeout = 3000;
                    dialPort = ((IPEndPoint)udp.Client.LocalEndPoint).Port;
                    var a = psnpn1117r2_Ż_ù__Links_Across_the_Internet.Ask(udp, stun[0]);
                    var b = psnpn1117r2_Ż_ù__Links_Across_the_Internet.Ask(udp, stun[1]);
                    if (a == null || b == null || !a.Equals(b)) throw new Exception();
                    mine = a;
                }
                Line("public", mine.ToString());

                step = "reach";
                var at = Post("mesh/reach.php", "address=" + mine.Address + "&port=" + mine.Port);
                var server = new IPEndPoint(IPAddress.Parse((string)at["address"]), Convert.ToInt32(at["port"]));
                Line("reach", server.ToString());

                step = "seen";
                var fresh = new byte[32];
                using (var rng = RandomNumberGenerator.Create()) rng.GetBytes(fresh);
                var ask = Encoding.UTF8.GetBytes(Json.Serialize(new Dictionary<string, object> { { "fresh", Convert.ToBase64String(fresh) } }));
                var reply = Parse(psnpn1117r2_quic.Ask(server, dialPort, psnpn1117r2_Table_Ż_1__reach, ask));
                var statement = psnpn1117r2_open_statement(reply, "reach", Convert.FromBase64String(psnpn1117r2_server_key));
                if (statement == null || (string)statement["fresh"] != Convert.ToBase64String(fresh)) throw new Exception();
                Line("seen", (string)statement["seen"]);

                Line("is_yes", "yes");
                return 0;
            }
            catch (Exception)
            {
                Console.WriteLine(step);
                Line("is_yes", "no");
                return 1;
            }
        }

        static void Line(string label, string value)
        {
            Console.WriteLine(label.PadRight(8) + value);
        }

        static Dictionary<string, object> Parse(byte[] utf8)
        {
            return Json.DeserializeObject(Encoding.UTF8.GetString(utf8)) as Dictionary<string, object>;
        }

        static Dictionary<string, object> Get(string path)
        {
            var req = (HttpWebRequest)WebRequest.Create(psnpn1117r2_API1 + path);
            return Answer(req);
        }

        static Dictionary<string, object> Post(string path, string form)
        {
            var req = (HttpWebRequest)WebRequest.Create(psnpn1117r2_API1 + path);
            req.Method = "POST";
            req.ContentType = "application/x-www-form-urlencoded";
            req.Timeout = 30000;
            var body = Encoding.ASCII.GetBytes(form);
            using (var s = req.GetRequestStream()) s.Write(body, 0, body.Length);
            return Answer(req);
        }

        static Dictionary<string, object> Answer(HttpWebRequest req)
        {
            using (var res = req.GetResponse())
            using (var s = res.GetResponseStream())
            using (var ms = new MemoryStream())
            {
                s.CopyTo(ms);
                return Parse(ms.ToArray());
            }
        }

        static Dictionary<string, object> psnpn1117r2_open_statement(Dictionary<string, object> statement, string kind, byte[] point)
        {
            if (statement == null || point.Length != 65 || point[0] != 4) return null;
            var signed = Convert.FromBase64String((string)statement["signed"]);
            var sig = Convert.FromBase64String((string)statement["sig"]);
            if (sig.Length != 64) return null;
            var p = new ECParameters
            {
                Curve = ECCurve.NamedCurves.nistP256,
                Q = new ECPoint { X = point.Skip(1).Take(32).ToArray(), Y = point.Skip(33).ToArray() },
            };
            using (var ec = ECDsa.Create(p)) if (!ec.VerifyData(signed, sig, HashAlgorithmName.SHA256)) return null;
            var body = Parse(signed);
            return body != null && (body["kind"] as string) == kind ? body : null;
        }
    }
}
