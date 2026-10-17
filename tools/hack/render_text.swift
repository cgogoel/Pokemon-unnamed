// Renders a line of text to a grayscale coverage map (PGM) for tools/hack/make_title_logo.py.
// usage: swift render_text.swift <text> <font PostScript name> <point size> <out.pgm>
import CoreGraphics
import CoreText
import Foundation

let args = CommandLine.arguments
let text = args[1], fontName = args[2], size = CGFloat(Double(args[3])!), out = args[4]
let font = CTFontCreateWithName(fontName as CFString, size, nil)
let attrs: [NSAttributedString.Key: Any] = [NSAttributedString.Key(kCTFontAttributeName as String): font,
    NSAttributedString.Key(kCTForegroundColorAttributeName as String): CGColor(gray: 1, alpha: 1)]
let line = CTLineCreateWithAttributedString(NSAttributedString(string: text, attributes: attrs))
let bounds = CTLineGetImageBounds(line, nil)
let pad = 4
let w = Int(ceil(bounds.width)) + pad * 2, h = Int(ceil(bounds.height)) + pad * 2
let ctx = CGContext(data: nil, width: w, height: h, bitsPerComponent: 8, bytesPerRow: w,
                    space: CGColorSpaceCreateDeviceGray(), bitmapInfo: CGImageAlphaInfo.none.rawValue)!
ctx.setFillColor(gray: 0, alpha: 1)
ctx.fill(CGRect(x: 0, y: 0, width: w, height: h))
ctx.textPosition = CGPoint(x: CGFloat(pad) - bounds.minX, y: CGFloat(pad) - bounds.minY)
CTLineDraw(line, ctx)
let data = ctx.data!.bindMemory(to: UInt8.self, capacity: w * h)
var pgm = "P5\n\(w) \(h)\n255\n".data(using: .ascii)!
pgm.append(Data(bytes: data, count: w * h))
try! pgm.write(to: URL(fileURLWithPath: out))
