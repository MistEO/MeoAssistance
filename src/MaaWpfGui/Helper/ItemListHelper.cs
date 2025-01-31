// <copyright file="ItemListHelper.cs" company="MaaAssistantArknights">
// MaaWpfGui - A part of the MaaCoreArknights project
// Copyright (C) 2021 MistEO and Contributors
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License v3.0 only as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY
// </copyright>

#nullable enable

using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MaaWpfGui.Constants;
using MaaWpfGui.Models;
using Serilog;

namespace MaaWpfGui.Helper
{
    public static class ItemListHelper
    {
        public static Dictionary<string, ArkItem> ArkItems { get; }

        private static readonly ILogger _logger = Log.ForContext("SourceContext", "ItemListHelper");

        static ItemListHelper()
        {
            var language = ConfigurationHelper.GetGlobalValue(ConfigurationKeys.Localization, LocalizationHelper.DefaultLanguage);
            string filename = string.Empty;
            switch (language)
            {
                case "zh-cn":
                    filename = Path.Combine(Directory.GetCurrentDirectory(), "resource", "item_index.json");
                    break;

                case "pallas":
                    break;

                default:
                    filename = Path.Combine(Directory.GetCurrentDirectory(), "resource", "global", DataHelper.ClientDirectoryMapper[language], "resource", "item_index.json");
                    break;
            }

            var tempItems = new Dictionary<string, ArkItem>();

            if (File.Exists(filename) is false)
            {
                ArkItems = tempItems;
                _logger.Warning("Item list file not found: {Filename}", filename);
                return;
            }

            try
            {
                _logger.Information("Loading item list from {filename}", filename);
                var jsonStr = File.ReadAllText(filename);
                tempItems = JsonSerializer.Deserialize<Dictionary<string, ArkItem>>(jsonStr);
            }
            catch (Exception e)
            {
                _logger.Error(e, "Failed to load item list from {filename}", filename);
            }

            ArkItems = tempItems ?? new Dictionary<string, ArkItem>();
        }

        /// <summary>
        /// 获取当前语言下的物品名称 / Get the name of the item in the current language
        /// </summary>
        /// <param name="itemId">物品 id / Item id</param>
        /// <returns>物品名称</returns>
        public static string GetItemName(string itemId)
        {
            return ArkItems.TryGetValue(itemId, out var item)
                ? item.Name
                : itemId;
        }

        /// <summary>
        /// 获取对应物品的图标 / Get the icon of the corresponding item
        /// </summary>
        /// <param name="itemId">物品 id / Item id</param>
        /// <returns>物品图片</returns>
        public static BitmapImage? GetItemImage(string itemId)
        {
            var imagePath = Path.Combine(Environment.CurrentDirectory, $"resource/template/items/{itemId}.png");
            if (!File.Exists(imagePath))
            {
                return null;
            }

            try
            {
                var bitmapImage = new BitmapImage(new(imagePath, UriKind.RelativeOrAbsolute));

                var stride = bitmapImage.PixelWidth * ((bitmapImage.Format.BitsPerPixel + 7) / 8);
                var pixelData = new byte[stride * bitmapImage.PixelHeight];
                bitmapImage.CopyPixels(pixelData, stride, 0);

                // 把黑边变成透明（其实是所有的黑色都变成透明了x
                for (int i = 0; i < pixelData.Length; i += 4)
                {
                    if (pixelData[i] == 0 && pixelData[i + 1] == 0 && pixelData[i + 2] == 0)
                    {
                        pixelData[i + 3] = 0;
                    }
                }

                // 不知道为啥 WriteableBitmap(bitmapImage); 得到的图没有透明度，大概是原始 uri 对应的是个 24 位的图？手动处理一下
                var writeableBitmap = new WriteableBitmap(bitmapImage.PixelWidth, bitmapImage.PixelHeight, bitmapImage.DpiX, bitmapImage.DpiY, PixelFormats.Bgra32, null);
                writeableBitmap.WritePixels(new(0, 0, bitmapImage.PixelWidth, bitmapImage.PixelHeight), pixelData, stride, 0);

                bitmapImage = new();
                using MemoryStream stream = new MemoryStream();
                PngBitmapEncoder encoder = new PngBitmapEncoder();
                encoder.Frames.Add(BitmapFrame.Create(writeableBitmap));
                encoder.Save(stream);
                bitmapImage.BeginInit();
                bitmapImage.CacheOption = BitmapCacheOption.OnLoad;
                bitmapImage.StreamSource = stream;
                bitmapImage.EndInit();
                bitmapImage.Freeze();
                return bitmapImage;
            }
            catch
            {
                return null;
            }
        }
    }
}
