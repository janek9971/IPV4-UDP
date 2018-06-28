using HtmlAgilityPack;
using System;
using System.Collections;
using System.Data;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace GPWreader
{

    class Program
    {

        static void Main(string[] args)
        {

            Console.OutputEncoding = System.Text.Encoding.UTF8;
            HtmlWeb web = new HtmlWeb();
            web.OverrideEncoding = Encoding.GetEncoding("ISO-8859-2");

            var doc = web.Load(@"https://www.money.pl/gielda/indeksy_gpw/wig_banki/");


            ArrayList bankNamesAlWeb = new ArrayList();
            ArrayList columnsNamesAlWeb = new ArrayList();




            if (doc != null)
            {

                var columnsNames = doc.DocumentNode.SelectNodes(" /html/body/div[1]/div[2]/div[2]/div[6]/div/div[2]/table/thead/tr/ th ");

                var bankNames = doc.DocumentNode.SelectNodes("  / html / body / div[1] / div[2] / div[2] / div[6] / div / div[2] / table / tbody / tr / td / a");


                foreach (var node in columnsNames)
                {

                    columnsNamesAlWeb.Add(node.InnerText);

                    Console.WriteLine(columnsNames.ToString());


                }



                foreach (var node in bankNames)
                {
                    bankNamesAlWeb.Add(node.InnerText);

                }

            }
            string[] columnsNamesArray = (string[])columnsNamesAlWeb.ToArray(typeof(string));
            try
            {
                //  SqlConnectionStringBuilder builder = ConnectionString();
                using (SqlConnection connection = new SqlConnection(ConnectionString().ConnectionString))
                {
                    Console.WriteLine("\nQuery data example:");
                    Console.WriteLine("=========================================\n");

                    StringBuilder sb = new StringBuilder();


                    //  sb.Append("SELECT CASE WHEN OBJECT_ID('dbo.WIG_BANKS', 'U') IS NOT NULL THEN 1 ELSE 0 END ");


                    if (!checkTableExists())
                    {
                        sb.Append("CREATE TABLE ");
                        sb.Append("@table");
                        sb.Append(" (");

                        for (int i = 0; i < columnsNamesArray.Length; i++)
                        {
                            //  if (!String.IsNullOrEmpty(terms2[i]))
                            // {


                            columnsNamesArray[i] = string.Join(" ", columnsNamesArray[i].Split().Where(x => x != ""));
                            columnsNamesArray[i] = columnsNamesArray[i].Replace(" ", "_");
                            columnsNamesArray[i] = columnsNamesArray[i].Replace("%", string.Empty);
                            columnsNamesArray[i] = columnsNamesArray[i].Replace("[", string.Empty);
                            columnsNamesArray[i] = columnsNamesArray[i].Replace("]", string.Empty);

                            sb.Append(columnsNamesArray[i]);

                            sb.Append(" ");
                            sb.Append("VARCHAR(20)");
                            sb.Append(", ");
                            // }
                        }

                        if (columnsNamesArray.Length > 1) { sb.Length -= 2; }  //Remove trailing ", "
                        sb.Append(") ");
                    }
                    //      sb.Append("CREATE TABLE WIG_BANKS (BankName VARCHAR(20), score INT)");


                    //ważne
                    /* sb.Append("insert into WIG_BANKS (");
                     for (int i = 0; i < terms2.Length; i++)
                     {
                         sb.Append(terms2[i]);
                     }*/
                    //   sb.Append("insert into WIG_BANKS (Nazwa_spółki) values (@BankNames)");

                    String sql = sb.ToString();
                    Console.WriteLine(sql);
                    using (SqlCommand command = new SqlCommand(sql, connection))
                    {
                        connection.Open();
                        if (!checkTableExists())
                            command.ExecuteNonQuery();
                        command.Parameters.Add("@table", SqlDbType.NVarChar).Value = Globals.tblName;
                        /*   for(int i=0; i<Globals.columnsNamesArrayDB.Length; i++)
                           {
                               command.Parameters.Add(Globals.columnsNamesArrayDB[i], SqlDbType.VarChar);
                           }*/
                        command.Parameters.Add(Globals.columnsNamesArrayDB[0], SqlDbType.VarChar);


                        command.CommandText = @"insert into WIG_BANKS (Nazwa_spółki) values (@BankNames)";



                        for (int i = 0; i < bankNamesAlWeb.Count; i++)
                        {
                            Console.WriteLine(bankNamesAlWeb[i].ToString());
                            command.Parameters["@BankNames"].Value = bankNamesAlWeb[i].ToString();
                            if (i != bankNamesAlWeb.Count - 1)
                                command.ExecuteNonQuery();


                        }

                        Console.WriteLine("=========================================\n");



                        /*   for (int i = 0; i < terms.Length; i++)
                           {
                               if (!String.IsNullOrEmpty(terms[i]))
                               {
                                   Console.WriteLine(terms[i]);
                                   command.Parameters["@BankNames"].Value = terms[i];
                                   if(i!=terms.Length-1)
                                   command.ExecuteNonQuery();
                               }
                           }*/
                        using (SqlDataReader reader = command.ExecuteReader())
                        {
                            while (reader.Read())
                            {
                                Console.WriteLine("{0} {1}", reader.GetString(0), reader.GetString(1));
                            }
                        }
                    }
                }
            }
            catch (SqlException e)
            {
                Console.WriteLine(e.ToString());
            }


            Console.ReadLine();
        }
        static SqlConnectionStringBuilder ConnectionString()
        {
            SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder();
            builder.DataSource = "gpwalayzer.database.windows.net";
            builder.UserID = "GPWadminLogin";
            builder.Password = "GPWloginPassword1337";
            builder.InitialCatalog = "GPWbankDB";
            return builder;
        }
        static bool comparision()
        {

            var query1 = "Select * from table1 order by recordId";
            var connection1 = new SqlConnection(ConnectionString().ConnectionString);
            var cmd1 = new SqlCommand(query1, connection1);
            var reader1 = cmd1.ExecuteReader();

            var query2 = "Select * from table2 order by recordId";
            var connection2 = new SqlConnection(ConnectionString().ConnectionString);
            var cmd2 = new SqlCommand(query2, connection2);
            var reader2 = cmd2.ExecuteReader();


            return true;
        }
        static bool checkTableExists()
        {
            int exists = 0;
            SqlConnectionStringBuilder builder = ConnectionString();


            string str = @"IF EXISTS(
                SELECT 1 FROM INFORMATION_SCHEMA.TABLES 
                WHERE TABLE_NAME = @table) 
                SELECT 1 ELSE SELECT 0";

            try
            {
                using (SqlConnection connection = new SqlConnection(ConnectionString().ConnectionString))
                {
                    using (SqlCommand command = new SqlCommand(str, connection))
                    {
                        connection.Open();
                        SqlCommand cmd = new SqlCommand(str, connection);

                        cmd.Parameters.Add("@table", SqlDbType.NVarChar).Value = Globals.tblName;
                        exists = (int)cmd.ExecuteScalar();

                        connection.Close();
                    }
                }
            }
            catch (SqlException ex)
            {
                Console.WriteLine("Check table SqlException" + ex);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Check table Exception" + ex);

            }
            if (exists == 1)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        static String parseDiacritic(String str)
        {

            //str = str.ToLower();

            var strArray = str.ToCharArray();

            for (int i = 0; i < strArray.Length; ++i)
            {

                switch (strArray[i])
                {

                    case 'ą':
                        strArray[i] = 'a';
                        break;
                    case 'ć':
                        strArray[i] = 'c';
                        break;
                    case 'ę':
                        strArray[i] = 'e';
                        break;
                    case 'ł':
                        strArray[i] = 'l';
                        break;
                    case 'ń':
                        strArray[i] = 'n';
                        break;
                    case 'ó':
                        strArray[i] = 'o';
                        break;
                    case 'ś':
                        strArray[i] = 's';
                        break;
                    case 'ź':
                        strArray[i] = 'z';
                        break;
                    case 'ż':
                        strArray[i] = 'z';
                        break;

                }

            }

            str = new string(strArray);

            return str;

        }

    }
    public static class Globals
    {
        public static String[] columnsNamesArrayDB = { "@BankNames", "@Pack", "@ShareInPortfolio", "@ShareOfTurnover", "@ImpactOnChangeOfIndex", "@ExchangeRate", "@PLN_ExchangeRate" };

        public static readonly String tblName = @"WIG_BANKS"; // Unmodifiable
    }
}
